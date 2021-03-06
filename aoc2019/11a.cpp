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



    void set(int x, int y, int value){
        buffer[y * W + x] = value;
        visited[y * W + x] = 1;
    }

    int get(int x, int y){
        return buffer[y * W + x];
    }

    // void print() {
    //     for( int j = 0; j < H; j++){
    //         for( int i = 0; i < W; i++){
    //             std::cout << c.get(i, j) << ;
    //         }
    //     }
    // }
    // void set(int pos, int value){
    //     buffer[pos] = value;
    // }

    // int get(int pos){
    //     return buffer[pos];
    // }

};


int mode(int opcode, int rParam){
    int modif = opcode / 100;
    int tenP = std::pow(10, rParam - 1);

    return (modif / tenP) % 10;
}

int DIRS[4][2] = {
    {0, -1},
    {1, 0},
    {0, 1},
    {-1, 0}
};

struct Computer{
    int status; //Blocked = 1, Halted = 2.
    int relativeBase;
    int pp;
    __int64* ram;
    std::deque<__int64>* out_pin;

    Canvas canvas;
    int px, py;
    int dir;
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

// void in(Computer* m){
//     if( m->in_pin.size() == 0 ){
//         m->status = 1; //Blocked
//         return;
//     }

//     setParam(m, 1, m->in_pin.front());
//     m->in_pin.pop_front();

//     m->pp += 2;
//     m->status = 0;
// }

void inRobot(Computer* m){
    setParam(m, 1, m->canvas.get(m->px, m->py));
    m->pp += 2;
    m->status = 0;
}

void out(Computer *m){
    __int64 a = getParam(m, 1);
    m->out_pin->push_back(a);
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
    inRobot,//in,
    out,
    jnz,
    jz,
    lt,
    eq,
    rel
};


int painted = 0;

void paintMove( Computer * m , int paint, int move){
    //std::cout << "Paint: " << paint << ", " << move << std::endl;
    m->canvas.set(m->px, m->py, paint);

    int dirDelta = 2 * move - 1;
    m->dir += dirDelta;

    if( m->dir == -1 )
        m->dir = 3;
    
    if( m->dir == 4 )
        m->dir = 0;

    //advance 
    m->px += DIRS[m->dir][0];
    m->py += DIRS[m->dir][1];

}

int run(Computer* m){
    do{
        if( m->ram[m->pp] == 99 ){
            m->status = 2; //halted
            break;
        }
        op[ m->ram[m->pp] % 100 ](m);

        if( m->out_pin->size() == 2 ){
            int paint = m->out_pin->front(); 
            m->out_pin->pop_front();

            int move = m->out_pin->front();
            m->out_pin->pop_front();

            paintMove(m, paint, move);
        }

    }while( m->status == 0 );
    return m->status;
}

char DIR_C[4] = {'^', '>', 'v', '<'};
char COL[2] = {'.', '#'};

void debug(Computer * m){

    for( int j = 0; j < m->canvas.H; j++){
        for( int i = 0; i < m->canvas.W; i++){
            if( i == m->px && j == m->py ){
                std::cout << DIR_C[m->dir];
            }else{
                std::cout << COL [ m->canvas.get(i, j) ];
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
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
    for( int i = 0; i < c.W * c.H; i++){
        c.buffer[i] = 0;
        c.visited[i] = 0;
    }
    Computer m;
    std::deque<__int64> output;


    m.canvas = c;
    m.dir = 0;
    m.px = 500;
    m.py = 500;

    m.relativeBase = 0;
    m.ram = new __int64[ 1024 * 1024 ]; // Large Ram
    m.out_pin = &output;
    
    
    m.status = 0;
    m.pp = 0;
    std::copy(rom.begin(), rom.end(), m.ram);

    int retCode = run(&m);

    std::cout << "Should be 0: " << output.size() << std::endl;

    // for( int i = 0; i < output.size(); i++){
    //      std::cout << output[i];
    // }
    // debug(&m);
    // paintMove(&m, 1, 0);
    // debug(&m);
    // paintMove(&m, 0, 0);
    // debug(&m);
    // paintMove(&m, 1, 0);
    // debug(&m);
    // paintMove(&m, 1, 0);
    // debug(&m);
    // paintMove(&m, 0, 1);
    // debug(&m);
    // paintMove(&m, 1, 0);
    // debug(&m);
    // paintMove(&m, 1, 0);
    // debug(&m);


    int count = 0;
    for( int i = 0; i < c.W * c.H; i++){
        count += c.visited[i];
    }
    // for( int i = 0; i < c.W; i++){
    //     for( int j = 0; j < c.H; j++){
    //         count += c.get(i, j);
    //     }
    // }

    std::cout << count << std::endl;

    
    return 0;
}