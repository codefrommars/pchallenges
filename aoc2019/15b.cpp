#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <string>

struct Canvas {
    int* buffer;
    int* visited;
    int W;
    int H;

    int minx;
    int miny;
    int maxx;
    int maxy;


    void set(int x, int y, int value){
        buffer[y * W + x] = value;
        visited[y * W + x] = 1;
        if( x < minx )
            minx = x;
        if( x > maxx )
            maxx = x;
        if( y < miny )
            miny = y;
        if( y > maxy )
            maxy = y;
    }

    void setNew(int x, int y, int value){
        if( get(x, y) == -1 )
            set(x, y, value);
    }

    int get(int x, int y){
        return buffer[y * W + x];
    }

};


int mode(int opcode, int rParam){
    int modif = opcode / 100;
    int tenP = std::pow(10, rParam - 1);

    return (modif / tenP) % 10;
}



struct Computer{
    int status; //Blocked = 1, Halted = 2.
    int relativeBase;
    int pp;
    __int64* ram;
    //std::deque<__int64>* out_pin;

    //Problem specific
    int in_pin;
    int out_pin;
    Canvas* canvas;
    int px, py;

};

__int64 getParam(Computer * m, int pIndex){
    __int64 v = m->ram[m->pp + pIndex];
    int accessMode = mode(m->ram[m->pp], pIndex);

    switch( accessMode ){
        case 0 : //param mode
            return m->ram[ (size_t)v];
        
        case 1 : //immediate mode
            return v;
        
        case 2 : 
            return m->ram[(size_t)v + m->relativeBase];
    }
}

void setParam(Computer *m, int pIndex, __int64 value ){
    size_t c = m->ram[m->pp + pIndex];
    int accessMode = mode(m->ram[m->pp], pIndex);
    
    if( accessMode == 2 ){
        c += m->relativeBase;
    }

    m->ram[c] = value;
}


void add(Computer *m){

    __int64 a = getParam(m, 1);
    __int64 b = getParam(m, 2);
    setParam(m, 3, a + b);

    m->pp += 4;
}

void mul(Computer* m ){

    __int64 a = getParam(m, 1);
    __int64 b = getParam(m, 2);
    setParam(m, 3, a * b);

    m->pp += 4;
}

void findBallPaddle(Computer* m, int& ball, int& paddle ){
    Canvas c = *m->canvas;

    for( int j = c.miny; j <= c.maxy; j++){
        for( int i = c.minx; i <= c.maxx; i++){
            int tile = c.get(i, j);
            if( tile == 3 ){
                paddle = i;
            }else if( tile == 4 ){
                ball = i;
            }
            //std::cout << COL [ tile ];
        }
        //std::cout << std::endl;
    }
    //std::cout << "SCORE: " << m->score << std::endl<< std::endl;
}

void in(Computer* m){
    if( m->in_pin == 0 ){
        m->status = 1; //Blocked
        return;
    }

    setParam(m, 1, m->in_pin);
    

    m->pp += 2;
    m->status = 0;
}

int DIRS[5][2] = {
    {0, 0},
    {0, -1},
    {0, 1},
    {-1, 0},
    {1, 0},
};

int BACK[5] = { 0, 2, 1, 4, 3};

char COL[5] = {'#', '.', 'O', 'S'};


void out(Computer *m){
    __int64 a = getParam(m, 1);
    //m->out_pin->push_back(a);
    
    int dir = m->in_pin;

    int nx = m->px + DIRS[dir][0];
    int ny = m->py + DIRS[dir][1];

    // std::cout << nx  << ", " << ny << std::endl;
    //Draw to canvas
    switch( a ){ //Blocked couldn't move
        case 0:
            m->canvas->setNew(nx, ny, a);
            break;
        case 2:
            // std::cout << "Found 2" << std::endl;
        case 1:
            m->canvas->setNew( m->px, m->py, a);
            m->px = nx;
            m->py = ny;
            break;
    }
    
    m->out_pin = a;
    m->in_pin = 0;
    m->pp += 2;
}

void jnz(Computer *m){
    __int64 a = getParam(m, 1);
    __int64 b = getParam(m, 2);

    if( a != 0 )
        m->pp = b;
    else
        m->pp += 3;
}

void jz(Computer* m){
    __int64 a = getParam(m, 1);
    __int64 b = getParam(m, 2);

    if( a == 0 )
        m->pp = b;
    else
        m->pp += 3;
}

void lt(Computer* m){
    __int64 a = getParam(m, 1);
    __int64 b = getParam(m, 2);
    setParam(m, 3, a < b );
        
    m->pp += 4;
}

void eq(Computer* m){
    __int64 a = getParam(m, 1);
    __int64 b = getParam(m, 2);
    setParam(m, 3, a == b );

    m->pp += 4;
}

void rel(Computer* m){
    size_t a = (size_t)getParam(m, 1);
    m->relativeBase += a;
    m->pp += 2;
}

typedef void (*OPPointer)(Computer* ) ;

OPPointer op[] = {
    0,
    add,
    mul,
    in,//in,
    out,
    jnz,
    jz,
    lt,
    eq,
    rel
};


int painted = 0;

void paintTile( Computer * m , int x, int y, int t){
    m->canvas->set(x, y, t);

}

int run(Computer* m){
    do{
        if( m->ram[m->pp] == 99 ){
            m->status = 2; //halted
            break;
        }
        op[ m->ram[m->pp] % 100 ](m);

    }while( m->status == 0 );
    return m->status;
}



int countBlocks(Canvas c){
    int blocks = 0;
    for( int j = c.miny; j <= c.maxy; j++){
        for( int i = c.minx; i <= c.maxx; i++){
            if( c.get(i, j) == 2 )
                blocks++;
        }
    }
    return blocks;
}

int checkDir(Computer* m, int dir){
    m->in_pin = dir;
    run(m);
    return m->out_pin;
}

void explore(Computer* m, int backDir, int val, int dst){
    Canvas c = *(m->canvas);
    //Where I'm currently in should be walkable
    c.setNew(m->px, m->py, val);
    // for( int j = c.miny; j <= c.maxy; j++){
    //     for( int i = c.minx; i <= c.maxx; i++){
    //         std::cout << COL [ c.get(i, j) ];
    //     }
    //     std::cout << std::endl;
    // }
    // std::cout << std::endl;

    for( int dir = 1; dir <= 4; dir++){
        if( dir == backDir )
            continue;

        int v = checkDir(m, dir);

        if( v != 0 ){
            // if( v == 2 ){
            //     //std::cout << "Found at dst: "  << (dst + 1) << std::endl;
            // }

            explore(m, BACK[dir], v, dst + 1);
        }
    }
    //Go back.
    if( backDir != 0 ){
        checkDir(m, backDir);
    }
}

bool contains(std::deque<int> *q, int index){
    for(int j = 0; j < q->size(); j++)
        if( q->at(j) == index )
            return true;

    return false;
}

void pushNeighbors(Canvas* c, std::deque<int> *q, int x, int y){
    for(int i = 1; i <= 4; i++){
        int nx = x + DIRS[i][0];
        int ny = y + DIRS[i][1];

        if( c->get(nx, ny) == 1 ){
            int index = nx + 1000 * ny;
            if( !contains(q, index ) )
                q->push_back(index);
        }
    }
}

int fill(Canvas* c, int ox, int oy){
    std::deque<int> q;
    
    pushNeighbors(c, &q, ox, oy);
    int time = 0;

    while(q.size() > 0 ){
        int s = q.size();
        time++;
        for(int i = 0; i < s; i++){
            int n = q.front();
            q.pop_front();
            int nx = n % 1000;
            int ny = n / 1000;
            c->set(nx, ny, 2);
            pushNeighbors(c, &q, nx, ny);
        }
    }

    return time;
}

int main(){

    std::vector<__int64> rom;

    for (__int64 i; std::cin >> i;) {
        rom.push_back(i);
        if (std::cin.peek() == ',')
            std::cin.ignore();
        else
        {
            break;
        }
    }

    Canvas c;
    c.W = 1000;
    c.H = 1000;
    c.buffer = new int[c.W * c.H];
    c.visited = new int[c.W * c.H];
    c.minx = 999999;
    c.maxx = -1;
    c.miny = 999999;
    c.maxy = -1;

    for( int i = 0; i < c.W * c.H; i++){
        c.buffer[i] = -1;
        c.visited[i] = 0;
    }
    Computer m;
    //std::deque<__int64> output;


    m.canvas = &c;
    m.px = 500;
    m.py = 500;
    //c.set(500, 500, 1);

    m.relativeBase = 0;
    m.ram = new __int64[ 1024 * 1024 ]; // Large Ram
    std::copy(rom.begin(), rom.end(), m.ram);

    m.in_pin = 0;
    //m.out_pin = &output;
    
    
    m.status = 0;
    m.pp = 0;

    explore(&m, 0, 1, 0);

    int ox, oy;
    for( int j = c.miny; j <= c.maxy; j++){
        for( int i = c.minx; i <= c.maxx; i++){
            if( c.get(i, j) == 2 ){
                ox = i;
                oy = j;
            }
            // std::cout << COL [ c.get(i, j) ];
        }
        // std::cout << std::endl;
    }

    

    int time = fill(&c, ox, oy);
    for( int j = c.miny; j <= c.maxy; j++){
        for( int i = c.minx; i <= c.maxx; i++){
            std::cout << COL [ c.get(i, j) ];
        }
        std::cout << std::endl;
    }

    std::cout << time << std::endl;


    // m.ram[0] = 2;

    // do{
    //     run(&m);
    // }while( countBlocks(c) > 0 );
    
    //  std::cout << m.score << std::endl;
    
    
    return 0;
}