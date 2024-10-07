#include "kruskal.h"
#include "graph.h"
#include <vector>
#include <cstdio> //sscanf

class  Edge {
	public:
		Edge(size_t id1=0, size_t id2=0, float weight=0) : id1(id1), id2(id2), weight(weight) {}
		size_t ID1() const { return id1; }
		size_t ID2() const { return id2; }
		float  Weight() const { return weight; }
		bool operator< ( Edge const& rhs) const { 
			return weight < rhs.weight
				|| ( weight == rhs.weight && id1 <  rhs.id1 )
				|| ( weight == rhs.weight && id1 == rhs.id1 && id2 < rhs.id2) ; 
		}
		friend std::ostream& operator<< (std::ostream& os, Edge const& e) {
			os << "(" << e.id1 << " -> " << e.id2 << ")";
			return os;
		}
	private:
		size_t id1;
		size_t id2;
		float weight; 
};

class Vertex {
	public:
		Vertex( size_t _id=0 ) : id(_id) {}
		size_t ID() const { return id; }
		bool operator< ( Vertex const& rhs) const { return id < rhs.id; }
		friend std::ostream& operator<< (std::ostream& os, Vertex const& v) {
			os << "Vertex(" << v.id << " )";
			return os;
		}
	private:
	size_t id;
};

//disjoint subsets tests 0,...,10
void test0() {
	DisjointSets ds(10);
	for (int i=0; i<10; ++i ) ds.Make( );

	std::cout << ds << std::endl;

	for (int i=0; i<10; ++i ) 
		std::cout << i << " -> " << ds.GetRepresentative(i) << std::endl;
}

void test1() {
	DisjointSets ds(10);
	for (int i=0; i<10; ++i ) ds.Make( );

	ds.Join(1,0);
	std::cout << ds << std::endl;

	for (int i=0; i<10; ++i ) 
		std::cout << i << " -> " << ds.GetRepresentative(i) << std::endl;
}

void test2() {
	DisjointSets ds(10);
	for (int i=0; i<10; ++i ) ds.Make( );

	ds.Join(0,1);
	std::cout << ds << std::endl;

	for (int i=0; i<10; ++i ) 
		std::cout << i << " -> " << ds.GetRepresentative(i) << std::endl;
}

void test3() {
	DisjointSets ds(10);
	for (int i=0; i<10; ++i ) ds.Make( );

	ds.Join(0,1);
	ds.Join(0,2);
	ds.Join(0,3);
	ds.Join(0,4);
	std::cout << ds << std::endl;

	for (int i=0; i<10; ++i ) 
		std::cout << i << " -> " << ds.GetRepresentative(i) << std::endl;
}

void test4() {
	DisjointSets ds(10);
	for (int i=0; i<10; ++i ) ds.Make( );

	ds.Join(1,0);
	ds.Join(2,0);
	ds.Join(3,0);
	ds.Join(4,0);
	std::cout << ds << std::endl;

	for (int i=0; i<10; ++i ) 
		std::cout << i << " -> " << ds.GetRepresentative(i) << std::endl;
}

void test5() {
	DisjointSets ds(10);
	for (int i=0; i<10; ++i ) ds.Make( );

	ds.Join(1,0);
	ds.Join(0,2);
	ds.Join(3,0);
	ds.Join(0,4);
	std::cout << ds << std::endl;

	for (int i=0; i<10; ++i ) 
		std::cout << i << " -> " << ds.GetRepresentative(i) << std::endl;
}

void test6() {
	DisjointSets ds(10);
	for (int i=0; i<10; ++i ) ds.Make( );

	for (int i=1; i<10; ++i ) ds.Join(0,i);
	std::cout << ds << std::endl;

	for (int i=0; i<10; ++i ) 
		std::cout << i << " -> " << ds.GetRepresentative(i) << std::endl;
}

void test7() {
	DisjointSets ds(10);
	for (int i=0; i<10; ++i ) ds.Make( );

	for (int i=1; i<10; ++i ) ds.Join(i,0);
	std::cout << ds << std::endl;

	for (int i=0; i<10; ++i ) 
		std::cout << i << " -> " << ds.GetRepresentative(i) << std::endl;
}

void test8() {
	DisjointSets ds(10);
	for (int i=0; i<10; ++i ) ds.Make( );

	for (int i=1; i<10; i+=2 ) { 
		ds.Join(i,0); 
		ds.Join(0,i); 
	}
	std::cout << ds << std::endl;

	for (int i=0; i<10; ++i ) 
		std::cout << i << " -> " << ds.GetRepresentative(i) << std::endl;
}

void test9() {
	DisjointSets ds(10);
	for (int i=0; i<10; ++i ) ds.Make( );

	for (int i=1; i<10; i+=2 ) {
		ds.Join(i,0);
		ds.Join(0,i);
	}
	std::cout << ds << std::endl;

	for (int i=0; i<10; ++i ) 
		std::cout << i << " -> " << ds.GetRepresentative(i) << std::endl;
}

void test10() {
	DisjointSets ds(10);
	for (int i=0; i<10; ++i ) ds.Make( );

	//test self-join
	ds.Join(0,1);
	ds.Join(0,1);
	ds.Join(1,0);
	std::cout << ds << std::endl;
	for (int i=0; i<10; ++i ) { 
		std::cout << i << " -> " << ds.GetRepresentative(i) << std::endl;
	}
}

// read from file
#include <fstream>
void solve_from_file( char const * filename )
{
    std::ifstream in( filename ); // closed automatically
    if ( in.fail() ) throw "Cannot open input file";

    // read problem
    int V,M;
    in >> V >> M;

	Graph<Vertex,Edge> g;
    
    // insert vertices
    for ( int i=0; i<V; ++i ) {
        g.InsertVertex( Vertex( i ) );
    }

    for ( int e=0;e<M;++e ) {
        size_t v1,v2,w;
        in >> v1 >> v2 >> w;
        g.InsertEdge( Edge( v1,v2,w ) );
        g.InsertEdge( Edge( v2,v1,w ) );
    }

    std::vector<Edge> mst = kruskal( g );

	std::sort( mst.begin(), mst.end() );

	std::vector<Edge>::const_iterator 
		it_edges     = mst.begin(),
		it_edges_end = mst.end();
	float length = 0.0f;

	for( ; it_edges != it_edges_end; ++it_edges ) {
		//std::cout << *it_edges << " ";
		length += it_edges->Weight();
	}
	std::cout << "  total length = " << length << std::endl;
    
}

void test11() { solve_from_file( "g5" ); }
void test12() { solve_from_file( "g5_2" ); }
void test13() { solve_from_file( "g5_3" ); }
void test14() { solve_from_file( "g10" ); }
void test15() { solve_from_file( "g500" ); }
void test16() { solve_from_file( "g1000" ); }

#include <numeric>      // iota
#include <algorithm>    // shuffle
#include <random>       // RNG
void random_graph( unsigned int N, Graph<Vertex,Edge> & g, unsigned int K ) 
{
    for ( unsigned int i=0; i<N; ++i ) {
        g.InsertVertex( Vertex( i ) );
    }
    
    // create a simple path 1-1-1-...-1-1-1, 
    // so that there is an MST of weight N-1
    for ( unsigned int i=0; i<N-1; ++i ) {
        g.InsertEdge( Edge( i,i+1,1 ) );
        g.InsertEdge( Edge( i+1,i,1 ) );
        //std::cout << i << " - " << i+1 << std::endl;
    }

    std::random_device                      rd;
    std::mt19937                            gen( rd() );
    std::uniform_int_distribution<int>      weight( 2, 10 ); // edge weight
    
    if ( K < N/2 ) { 
        // add random heavier edges
        std::vector<int> first2K( N ); 
        std::iota( first2K.begin(), first2K.end(), 0); // 0..N-1 
        std::shuffle( first2K.begin(), first2K.end(), std::mt19937{std::random_device{}()} );

        // shrink array to first2K to 2K elements
        first2K.resize( 2*K );

        for ( unsigned int i=0; i<2*K; i+=2 ) {
            // this edges will not be used in MST
            g.InsertEdge( Edge( first2K[i],first2K[i+1], weight( gen ) ) );
            g.InsertEdge( Edge( first2K[i+1],first2K[i], weight( gen ) ) );
        }
    }
}

void test17() {
    Graph<Vertex,Edge> g;
    random_graph( 1000000, g, 10 );
    std::vector<Edge> mst = kruskal( g );

	std::sort( mst.begin(), mst.end() );

	std::vector<Edge>::const_iterator 
		it_edges     = mst.begin(),
		it_edges_end = mst.end();
	float length = 0.0f;

	for( ; it_edges != it_edges_end; ++it_edges ) {
		//std::cout << *it_edges << " ";
		length += it_edges->Weight();
	}
	std::cout << "  total length = " << length << std::endl;
}


void (*pTests[])(void) = { 
    test0,test1,test2,test3,test4,test5,test6,
    test7,test8,test9,test10,test11,test12,
    test13,test14,test15,test16,test17
};

int main( int argc, char ** argv) {
	//if (argc!=2) return 1;
	//else {
	//	int test = 0;
	//	std::sscanf(argv[1],"%i",&test);
	//	pTests[ test ]();
	//}
	//return 0;
    int test = 0;
    if (argc != 2)
    {
        std::cin >> test;
    }
    else
    {
        std::sscanf(argv[1], "%i", &test);
    }
    pTests[test]();
    return 0;
}

