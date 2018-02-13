#include <transformation/cgraScheduler.h>
#include <lib/log.h>
#include <cstdio>

#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

int nbBlocs = 0;

typedef struct
{
	int32_t i;
	int32_t j;
} cgra_node;

cgra_node operator+(const cgra_node& l, const cgra_node& r)
{
	return { l.i+r.i, l.j+r.j };
}

bool operator<(const cgra_node& l, const cgra_node& r);

bool operator==(const cgra_node& l, const cgra_node& r)
{
	return l.i == r.i && l.j == r.j;
}


int manhatan(const cgra_node& src, const cgra_node& dst)
{
	return std::abs(src.i - dst.i)
			+ std::abs(src.j - dst.j);
}

class dist_from_deps
{
	static cgra_node _dep1;
	static cgra_node _dep2;
public:
	static void init(const cgra_node& dep1, const cgra_node& dep2)
	{
		_dep1 = dep1;
		_dep2 = dep2;
	}

	bool operator() (const cgra_node& a, const cgra_node& b)
	{
		if (_dep1.i == -1)
			return 0;

		int da = manhatan(a, _dep1), db = manhatan(b, _dep1);
		return da > db;
	}
};
cgra_node dist_from_deps::_dep1;
cgra_node dist_from_deps::_dep2;

typedef struct
{
	int16_t src1;
	int16_t src2;
} source;

///
/// \brief printGraph prints a xdot representation of the block's DFG
/// \param instructions: the basic block
/// \param numInstructions: its size
///
void printGraph(const uint128_struct * instructions, uint32_t numInstructions);

void sortByPriority(uint128_struct * instructions, uint32_t numInstructions);

///
/// \brief getNbDeps
/// \param i
/// \return number of WAW, RAW, and WAR dependencies of [i]
///
int getNbDeps(const uint128_struct &i);

///
/// \brief getLastDep
/// \param i
/// \return the last ID of an instruction using the same registers as [i]
///
int getLastDep(const uint128_struct &i);

///
/// \brief setupSources computes the edges of the basic block's DFG
/// \param instructions: the basic block
/// \param sources: the "from" edges of each instruction
/// \param numInstructions: the basic block's size
///
void setupSources(const uint128_struct * instructions, source * sources, uint32_t numInstructions);

bool route(const cgra_node& source, const cgra_node& target, uint32_t cgra_conf[3][4]);


void printConfig(uint32_t configuration[3][4]);

CgraScheduler::CgraScheduler()
{

}

void CgraScheduler::schedule(CgraSimulator& cgra, uint128_struct * instructions, uint32_t numInstructions)
{
	std::cout << "calling SCHEDULE\n";
	++nbBlocs;

	printGraph(instructions, numInstructions);

	uint32_t configuration[3][4] = {0};
	source * sources = new source[numInstructions];
	cgra_node * placeOfInstr = new cgra_node[numInstructions];

	setupSources(instructions, sources, numInstructions);

	for (uint32_t instrId = 0; instrId < numInstructions; ++instrId)
	{
		std::priority_queue<cgra_node, std::vector<cgra_node>, dist_from_deps> possible;
		uint128_struct instruction = instructions[instrId];
		bool routed = false; cgra_node place;

		const cgra_node noSrc = {-1,-1};
		dist_from_deps::init(
					sources[instrId].src1 != -1 ? placeOfInstr[sources[instrId].src1] : noSrc
				, sources[instrId].src2 != -1 ? placeOfInstr[sources[instrId].src2] : noSrc);

		// compute all available places, sorted by lexicographic order over
		// cgra's dimensions, starting by the closest layer to the VLIW
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (!configuration[i][j])
				{
					possible.push({ i, j });
				}
			}
		}

		// try to place the instruction in [place]
		// and route its operands
		uint32_t new_conf[3][4];
		while (!possible.empty())
		{
			routed = true;
			place = possible.top();
			possible.pop();
			std::cout << "trying place " << place.i << "; " << place.j << std::endl;

			std::memcpy(new_conf, configuration, 3*4*sizeof(uint32_t));
			new_conf[place.i][place.j] = instrId+2;

			if (sources[instrId].src1 != -1)
			{
				if (!(routed = route(placeOfInstr[sources[instrId].src1], place, new_conf)))
					continue;
			}

			if (sources[instrId].src2 != -1)
			{
				if (!(routed = route(placeOfInstr[sources[instrId].src2], place, new_conf)))
					continue;
			}

			if (routed)
				break;
		}

		// if all places fail, the scheduling is impossible
		if (!routed)
		{
			Log::fprintf(0, stderr, "CgraScheduler::schedule(): Couldn't schedule basic block %p", instructions);
			exit(-1);
		}
		else
		{
			placeOfInstr[instrId] = place;
			std::memcpy(configuration, new_conf, 3*4*sizeof(uint32_t));
			printConfig(configuration);
		}
	}

	delete placeOfInstr;
	delete sources;
}

void printConfig(uint32_t configuration[3][4])
{
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (configuration[i][j] == 0)
				Log::printf(0, " X ;");
			else if (configuration[i][j] < 20)
				Log::printf(0, "%3d;", configuration[i][j]-2);
			else
				Log::printf(0, " %c ;", configuration[i][j]);
		}
		Log::printf(0, "\n");
	}
}

int getLastDep(const uint128_struct& i)
{
	uint8_t deps[7];
	uint8_t ret = 0;

	deps[0] = i.word32 >> 16;
	deps[1] = i.word32 >> 8;
	deps[2] = i.word32 >> 0;
	deps[3] = i.word0 >> 24;
	deps[4] = i.word0 >> 16;
	deps[5] = i.word0 >> 8;
	deps[6] = i.word0 >> 0;

	for (int i = 0; i < 7; ++i)
	{
		ret = std::max(ret, deps[i]);
	}

	return ret;
}

void setupSources(const uint128_struct * instructions, source * sources, uint32_t numInstructions)
{
	for (uint32_t instrId = 0; instrId < numInstructions; ++instrId)
	{
		uint128_struct i = instructions[instrId];
		sources[instrId] = { -1, -1 };

		uint16_t virtualRDest = ((i.word64>>14) & 0x1ff);
		uint16_t virtualRIn2 = ((i.word64>>23) & 0x1ff);
		uint16_t virtualRIn1_imm9 = ((i.word96>>0) & 0x1ff);
		uint8_t opCode = ((i.word96>>19) & 0x7f);
		bool isImm = ((i.word96>>18) & 0x1);

		if ((opCode == VEX_STB || opCode == VEX_STD || opCode == VEX_STH || opCode == VEX_STW)
				&& virtualRDest < 256)
		{
			sources[instrId].src1 = virtualRDest;
		}
		else
		{
			if (virtualRIn2 < 256)
				sources[instrId].src1 = virtualRIn2;

			if (!isImm && virtualRIn1_imm9 < 256)
				sources[instrId].src2 = virtualRIn1_imm9;
		}

		std::cout << "deps[" << instrId << "] = " << sources[instrId].src1 << "; " << sources[instrId].src2 << "\n";
	}
}

int getNbDeps(const uint128_struct& i)
{
	return ((i.word64>>0) & 7);
}

int compare(const void * i1p, const void * i2p)
{
	uint128_struct i1, i2;
	i1 = *((uint128_struct*)i1p);
	i2 = *((uint128_struct*)i2p);
	return getLastDep(i1) - getLastDep(i2);
}

void sortByPriority(uint128_struct * instructions, uint32_t numInstructions)
{
	qsort(instructions, numInstructions, sizeof(uint128_struct), compare);
}

void printGraph(const uint128_struct *instructions, uint32_t numInstructions)
{
	FILE * f = fopen(std::string("/home/ablanleu/Documents/stage/xdot/cgra"+std::to_string(nbBlocs)+".dot").c_str(), "w");
	Log::fprintf(0, f, "digraph cgra {");
	for (uint32_t i = 0; i < numInstructions; ++i)
	{
		uint8_t opCode = ((instructions[i].word96>>19) & 0x7f);
		uint8_t typeCode = ((instructions[i].word96>>28) & 0x3);
		bool isImm = ((instructions[i].word96>>18) & 0x1);
		uint16_t src1 = ((instructions[i].word96>>0) & 0x1ff);
		uint16_t src2 = ((instructions[i].word64>>23) & 0x1ff);
		uint16_t dst  = ((instructions[i].word64>>14) & 0x1ff);

		Log::fprintf(0, f, "i%d [label=%s];", i, opcodeNames[opCode]);

		if (typeCode == 0)
		{
			if (opCode == VEX_STD || opCode == VEX_STW || opCode == VEX_STH || opCode == VEX_STB)
				if (dst < 256)
					Log::fprintf(0, f, "i%d -> i%d;", dst, i);
				else
					Log::fprintf(0, f, "r%d -> i%d;", dst-256, i);

			if (src2 < 256)
				Log::fprintf(0, f, "i%d -> i%d;", src2, i);
			else
				Log::fprintf(0, f, "r%d -> i%d;", src2-256, i);

			if (!isImm)
			{
				if (src1 < 256)
					Log::fprintf(0, f, "i%d -> i%d;", src1, i);
				else
					Log::fprintf(0, f, "r%d -> i%d;", src1-256, i);
			}
		}
	}
	Log::fprintf(0, f, "}");

	fclose(f);
}

class astar_node
{
	static astar_node _nodes[3][4];
	cgra_node _from;
	int _so_far;
	int _priority;
	bool _open;
	bool _closed;

public:

	static void init()
	{
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				_nodes[i][j]._from = { i, j };
				_nodes[i][j]._so_far = -1;
				_nodes[i][j]._priority = -1;
				_nodes[i][j]._open = 0;
				_nodes[i][j]._closed = 0;
			}
		}
	}

	static void init_node(const cgra_node& source
										 , bool open
										 , bool closed
										 , const cgra_node& from
										 , int so_far
										 , const cgra_node& target)
	{
		astar_node * n = &_nodes[source.i][source.j];
		n->_open = open;
		n->_closed = closed;
		n->_from = from;
		n->_so_far = so_far;
		n->_priority = so_far + manhatan(source, target);
	}

	static void open(const cgra_node& source, const cgra_node& neighbour, const cgra_node& target)
	{
		astar_node * n = &_nodes[source.i][source.j], * nei = &_nodes[neighbour.i][neighbour.j];
		n->_open = 1;
	}

	static void update(const cgra_node& source, const cgra_node& neighbour, const cgra_node& target)
	{
		astar_node * n = &_nodes[source.i][source.j], * nei = &_nodes[neighbour.i][neighbour.j];
		if (n->_so_far < 0 || (n->_so_far > nei->_so_far + 1))
		{
			n->_from = neighbour;
			n->_so_far = nei->_so_far + 10;
			n->_priority = n->_so_far + manhatan(source, target);
		}
	}

	static void close(const cgra_node& node)
	{
		_nodes[node.i][node.j]._closed = 1;
	}

	static void write_path(const cgra_node& target, uint32_t cgra_conf[3][4])
	{
		const cgra_node n = _nodes[target.i][target.j]._from;

		// write node
		if (cgra_conf[n.i][n.j] == 0)
		{
			char c;
			if (target.i > n.i)
				c = 'V';
			else if (target.i < n.i)
				c = '^';
			else if (target.j > n.j)
				c = '>';
			else if (target.j < n.j)
				c = '<';

			cgra_conf[n.i][n.j] = c;
		}

		// stop if start reached
		if (n == target)
			return;

		write_path(n, cgra_conf);
	}

	static bool is_open(const cgra_node& node)
	{
		return _nodes[node.i][node.j]._open;
	}

	static bool is_closed(const cgra_node& node)
	{
		return _nodes[node.i][node.j]._closed;
	}

	static int priority(const cgra_node& node)
	{
		return _nodes[node.i][node.j]._priority;
	}

};


static void print_state(uint32_t cgra_conf[3][4])
{
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (cgra_conf[i][j] == 0)
			{
				if (astar_node::is_closed({i,j}))
				{
					Log::printf(0, "%5d;", astar_node::priority({i,j}));
				}
				else
				{
					Log::printf(0, "  O  ;");
				}
			}
			else if (cgra_conf[i][j] < 20)
				Log::printf(0, "%5d;", cgra_conf[i][j]-2);
			else
				Log::printf(0, "  %c  ;", cgra_conf[i][j]);
		}
		Log::printf(0, "\n");
	}
}

astar_node astar_node::_nodes[3][4];

bool operator<(const cgra_node& l, const cgra_node& r)
{
	return astar_node::priority(l) < astar_node::priority(r);
}

///
/// \brief route using A* algorithm
/// \param source
/// \param target
/// \param cgra_conf
/// \return
///
bool route(const cgra_node& source, const cgra_node& target, uint32_t cgra_conf[3][4])
{
	// initialization of nodes+priority queue
	std::priority_queue<cgra_node, std::vector<cgra_node> > open;
	astar_node::init();

	// open = { start }
	open.push(source);
	astar_node::init_node(source, 1, 0, source, 0, target);

	while (!open.empty())
	{
		cgra_node current = open.top();

		// target reached, configure path
		if (current == target)
		{
			astar_node::write_path(target, cgra_conf);
			return true;
		}

		// don't visit node again
		open.pop();
		astar_node::close(current);

		// for each neighbour
		cgra_node a={0,1},b={1,0},c={0,-1},d={-1,0};
		for (cgra_node n : {a,b,c,d})
		{
			// position of neighbour
			cgra_node nn = n+current;

			if (nn.i >= 0 && nn.i < 3 && nn.j >= 0 && nn.j < 4)
			{
				// if neighbour already visited, do nothing
				if (astar_node::is_closed(nn) || (cgra_conf[nn.i][nn.j] != 0 && !(nn == target)))
					continue;

				if (!astar_node::is_open(nn))
				{
					open.push(nn);
					astar_node::open(nn, current, target);
				}

				astar_node::update(nn, current, target);
			}
		}
	}

	return false;
}
