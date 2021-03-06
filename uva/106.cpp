#include <cstdio>
#include <bitset>

#define MAX 1000001
std::bitset<MAX> p;

int gcd(int a, int b) {
    return b == 0 ? a : gcd(b, a % b);
}

int main(){
	int N;
	
	while( std::scanf("%i", &N) != EOF ){
		p.reset();
		
		int cop = 0;		
		//We are dealing with Primitive Pythagorean triplets
		// https://en.wikipedia.org/wiki/Pythagorean_triple#Generating_a_triple
		for( int m = 1; m * m <= N; m++ ){
			for(int n = m + 1; n * n <= N; n += 2 ){ //Both can't be odd at the same time
				if( gcd( m, n ) != 1 )
					continue;
				//We are making n > m  (different than in the wikipage)
				long long a = n * n - m * m;
				long long b = 2 * m * n;
				long long c = m * m + n * n;
				
				if( c > N )
					break;
				
				int k = 1;
				while( k * c <= N ){
					//std::printf("%lld <= %i\n", k * a, N);
					p.set(k * a);
					p.set(k * b);
					p.set(k * c);
					k++;
				}
				
				cop ++;
			}
		}
		std::printf("%i %i\n", cop, N - p.count() );
	}
}