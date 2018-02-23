#include <transformation/cgraScheduler.h>
#include <isa/cgraIsa.h>
#include <lib/log.h>
#include <cstdio>

#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

#define LOG_LO 0
#define LOG_HI 0

int II;
int currentII;

typedef struct
{
	int32_t i;
	int32_t j;
	int32_t k;
} cgra_node;

cgra_node operator+(const cgra_node& l, const cgra_node& r)
{
	return { l.i+r.i, l.j+r.j, l.k+r.k };
}

bool operator<(const cgra_node& l, const cgra_node& r);

bool operator==(const cgra_node& l, const cgra_node& r)
{
	return l.i == r.i && l.j == r.j && l.k == r.k;
}


int manhatan(const cgra_node& src, const cgra_node& dst)
{
	return std::abs(src.i - dst.i)
			+ std::abs(src.j - dst.j)
			+ std::abs(src.k - dst.k);
}

class dist_from_deps
{
	static cgra_node _dep1;
	static cgra_node _dep2;
	static uint64_t	 (*_cgra_conf)[3][4];
public:
	static void init(const cgra_node& dep1, const cgra_node& dep2, uint64_t (*cgra_conf)[3][4])
	{
		_dep1 = dep1;
		_dep2 = dep2;
		_cgra_conf = cgra_conf;
	}

	bool operator() (const cgra_node& a, const cgra_node& b)
	{
		if (a.k != b.k)
			return a.k > b.k;

		int da = 0, db = 0;
		if (_dep1.i != -1)
		{
			da = manhatan(a, _dep1);
			db = manhatan(b, _dep1);
		}
		if (_dep2.i != -1)
		{
			da += manhatan(a, _dep2);
			db += manhatan(b, _dep2);
		}

		if (da == db && a.k < currentII)
		{
			int freedoma = 0, freedomb = 0;
			const cgra_node a2={0,1,a.k+1},b2={1,0,a.k+1},c={0,-1,a.k+1},d={-1,0,a.k+1},e={0,0,a.k+1};
			for (cgra_node n : {a2,b2,c,d,e})
			{
				// position of neighbour
				cgra_node na = n+a;
				cgra_node nb = n+b;

				if (na.i >= 0 && na.i < 3 && na.j >= 0 && na.j < 4 && _cgra_conf[na.k][na.i][na.j] == 0) ++freedoma;
				if (nb.i >= 0 && nb.i < 3 && nb.j >= 0 && nb.j < 4 && _cgra_conf[nb.k][nb.i][nb.j] == 0) ++freedomb;
			}
			return freedoma < freedomb;
		}
		else
			return da > db;
	}
};

cgra_node dist_from_deps::_dep1;
cgra_node dist_from_deps::_dep2;
uint64_t (*dist_from_deps::_cgra_conf)[3][4];

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

///
/// \brief setupSources computes the edges of the basic block's DFG
/// \param instructions: the basic block
/// \param sources: the "from" edges of each instruction
/// \param numInstructions: the basic block's size
///
void setupSources(const uint128_struct * instructions, source * sources, uint32_t numInstructions);

bool route(const cgra_node& source, const cgra_node& target, uint64_t (*cgra_conf)[3][4], uint8_t *from);

int findII(uint128_struct * instructions, source * sources, uint32_t numInstructions)
{
	int16_t * depth = new int16_t[numInstructions];
	int16_t ret = 1;
	for (uint32_t i = 0; i < numInstructions; ++i)
	{
		depth[i] = 1;
	}

	for (uint32_t i = 0; i < numInstructions; ++i)
	{
		if (sources[i].src1 != -1)
			depth[i] = depth[sources[i].src1] + 1;
		if (sources[i].src2 != -1)
			depth[i] = std::max((int)depth[i], depth[sources[i].src2] + 1);

		ret = std::max(ret, depth[i]);
	}

	return ret;
}

bool canPlace(const FunctionalUnit& u, uint128_struct i)
{
	uint8_t opcode = (i.word96 >> 19) & 0x7f;
	uint16_t virtualRDest = ((i.word64>>14) & 0x1ff);
	uint16_t virtualRIn2 = ((i.word64>>23) & 0x1ff);
	uint16_t virtualRIn1_imm9 = ((i.word96>>0) & 0x1ff);
	bool isImm = ((i.word96>>18) & 0x1);
	bool ret = true;

	if (opcode == VEX_MPY)
		ret = ret && (u.features() & FunctionalUnit::FEATURE_MULT);

	if (opcode == VEX_STB || opcode == VEX_STD || opcode == VEX_STH || opcode == VEX_STW ||
			opcode == VEX_LDB || opcode == VEX_LDD || opcode == VEX_LDH || opcode == VEX_LDW)
	{
		ret = ret && (u.features() & FunctionalUnit::FEATURE_REG);
		if (virtualRDest >= 256)
		{
			ret = ret && (u.features() & FunctionalUnit::FEATURE_REG);
		}
	}

	if (virtualRIn2 >= 256)
	{
		ret = ret && (u.features() & FunctionalUnit::FEATURE_REG);
	}

	if (!isImm && virtualRIn1_imm9 >= 256)
	{
		ret = ret && (u.features() & FunctionalUnit::FEATURE_REG);
	}

	return ret;
}

CgraScheduler::CgraScheduler()
{

}

bool CgraScheduler::schedule(VexCgraSimulator& cgra, uint128_struct * instructions, uint32_t numInstructions)
{
	const FunctionalUnit * units = cgra.cgraSimulator.units();

	Log::out(0) << "Hello\n";
	//printGraph(instructions, numInstructions);

	source * sources = new source[numInstructions];
	cgra_node * placeOfInstr = (new cgra_node[numInstructions+1])+1;

	placeOfInstr[-1] = {-1,-1,-1};

	setupSources(instructions, sources, numInstructions);
	II = findII(instructions, sources, numInstructions);

	currentII = II;
	while (currentII != II*2)
	{
		// setup the scheduling space
		uint64_t (*configuration)[3][4] = new uint64_t[currentII][3][4];
		uint64_t (*new_conf)[3][4] = new uint64_t[currentII][3][4];
		for (int i = 0; i < currentII; ++i)
			for (int j = 0; j < 3; ++j)
				for (int k = 0; k < 4; ++k)
					configuration[i][j][k] = 0;

		bool routed = true;

		// for each instruction to schedule
		for (uint32_t instrId = 0; instrId < numInstructions; ++instrId)
		{
			if (currentII == II)
			{
				Log::out(0) << printBytecodeInstruction(0, instructions[instrId].word96, instructions[instrId].word64, instructions[instrId].word32, instructions[instrId].word0);
			}
			std::priority_queue<cgra_node, std::vector<cgra_node>, dist_from_deps> possible;
			uint128_struct instruction = instructions[instrId];
			uint8_t src1 = 0xff, src2 = 0xff;

			routed = false; cgra_node place;

			cgra_node n1 = placeOfInstr[sources[instrId].src1], n2 = placeOfInstr[sources[instrId].src2];

			dist_from_deps::init(
						n1
						, n2
						, configuration);

			static const cgra_node errNode = {-1,-1,-1};

			// compute all available places, sorted by priority given
			// by [dist_from_deps] class
			for (int k = std::max(n1.k, n2.k)+1; k < currentII; ++k)
			{
				for (int i = 0; i < 3; ++i)
				{
					for (int j = 0; j < 4; ++j)
					{
						int kk = 0;
						if (!(n1 == errNode))
						{
							kk = manhatan({i,j,n1.k},n1);
							if (!(n2 == errNode))
								kk = std::max(kk, manhatan({i,j,n2.k},n2));
						}

						if (k >= kk && !configuration[k][i][j] && canPlace(units[i*4+j], instruction))
						{
							possible.push({ i, j, k });
						}
					}
				}
			}

			// for each [place] in [possible], try to route the instruction's operands from its sources places to its place
			while (!possible.empty())
			{
				routed = true;
				place = possible.top();
				possible.pop();

				std::memcpy(new_conf, configuration, currentII*3*4*sizeof(uint64_t));
				new_conf[place.k][place.i][place.j] = instrId+2;

				// first route
				if (!(n1 == errNode))
				{
					if (!(routed = route(n1, place, new_conf, &src1)))
						continue;
				}

				// second route
				if (!(n2 == errNode))
				{
					if (!(routed = route(n2, place, new_conf, &src2)))
						continue;
				}

				if (routed)
					break;
			}

			// if all places fail, the scheduling is impossible
			if (!routed)
			{
				break;
			}
			else
			{
				placeOfInstr[instrId] = place;
				std::memcpy(configuration, new_conf, II*3*4*sizeof(uint64_t));
				configuration[place.k][place.i][place.j] = cgra::vex2cgra(instruction, src1, src2);
			}
		}

		// if we leave the scheduling loop with [routed == true]
		// then we managed to schedule the instructions into a CGRA configuration
		if (routed)
		{
			delete[] (placeOfInstr-1);
			delete[] sources;
			int id = cgra.configurationCache.size();
			cgra.configurationCache[id].configuration = (uint64_t*)configuration;
			cgra.configurationCache[id].cycles = currentII;

			instructions[0] = {0x3b << 19,id << 23,0,0};

			for (int i = 0; i < currentII; ++i)
			{
				Log::out(0) << "LAYER " << i << "\n";
				cgra::printConfig(0, (uint64_t*)(configuration[i]));
			}
			return true;
		}

		// if we haven't managed to schedule with this II, increase it and retry
		delete[] new_conf;
		currentII++;
	}

	// if we can't finish the schedule even with a big enough II
	// it is because some path is blocked. We don't handle this case.
	return false;
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

		if ((opCode == VEX_STB || opCode == VEX_STD || opCode == VEX_STH || opCode == VEX_STW))
		{
			if (virtualRIn2 < 256)
				sources[instrId].src1 = virtualRIn2;

			if (virtualRDest < 256)
				sources[instrId].src2 = virtualRDest;
		}
		else
		{
			if (virtualRIn2 < 256)
				sources[instrId].src1 = virtualRIn2;

			if (!isImm && virtualRIn1_imm9 < 256)
				sources[instrId].src2 = virtualRIn1_imm9;
		}

	}
}

void printGraph(const uint128_struct *instructions, uint32_t numInstructions)
{
	static int count = 0;

	++count;
	FILE * f = fopen(std::string("/home/ablanleu/Documents/stage/xdot/cgra"+std::to_string(count)+".dot").c_str(), "w");
	Log::fprintf(LOG_LO, f, "digraph cgra {");
	for (uint32_t i = 0; i < numInstructions; ++i)
	{
		uint8_t opCode = ((instructions[i].word96>>19) & 0x7f);
		uint8_t typeCode = ((instructions[i].word96>>28) & 0x3);
		bool isImm = ((instructions[i].word96>>18) & 0x1);
		uint16_t src1 = ((instructions[i].word96>>0) & 0x1ff);
		uint16_t src2 = ((instructions[i].word64>>23) & 0x1ff);
		uint16_t dst  = ((instructions[i].word64>>14) & 0x1ff);

		Log::fprintf(LOG_LO, f, "i%d [label=%s];", i, opcodeNames[opCode]);

		if (typeCode == 0)
		{
			if (opCode == VEX_STD || opCode == VEX_STW || opCode == VEX_STH || opCode == VEX_STB)
				if (dst < 256)
					Log::fprintf(LOG_LO, f, "i%d -> i%d;", dst, i);
				else
					Log::fprintf(LOG_LO, f, "r%d -> i%d;", dst-256, i);

			if (src2 < 256)
				Log::fprintf(LOG_LO, f, "i%d -> i%d;", src2, i);
			else
				Log::fprintf(LOG_LO, f, "r%d -> i%d;", src2-256, i);

			if (!isImm)
			{
				if (src1 < 256)
					Log::fprintf(LOG_LO, f, "i%d -> i%d;", src1, i);
				else
					Log::fprintf(LOG_LO, f, "r%d -> i%d;", src1-256, i);
			}
		}
	}
	Log::fprintf(LOG_LO, f, "}");

	fclose(f);
}

class astar_node
{
	static astar_node (*_nodes)[3][4];
	cgra_node _from;
	int _so_far;
	int _priority;
	bool _open;
	bool _closed;

public:

	static void init()
	{
		if (_nodes)
			delete[] _nodes;

		_nodes = new astar_node[II][3][4];
		for (int k = 0; k < II; ++k)
		{
			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					_nodes[k][i][j]._from = { i, j, k };
					_nodes[k][i][j]._so_far = -1;
					_nodes[k][i][j]._priority = -1;
					_nodes[k][i][j]._open = 0;
					_nodes[k][i][j]._closed = 0;
				}
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
		astar_node * n = &_nodes[source.k][source.i][source.j];
		n->_open = open;
		n->_closed = closed;
		n->_from = from;
		n->_so_far = so_far;
		n->_priority = so_far + manhatan(source, target);
	}

	static void open(const cgra_node& source, const cgra_node& neighbour, const cgra_node& target)
	{
		astar_node * n = &_nodes[source.k][source.i][source.j];
		n->_open = 1;
	}

	static void update(const cgra_node& source, const cgra_node& neighbour, const cgra_node& target)
	{
		astar_node * n = &_nodes[source.k][source.i][source.j], * nei = &_nodes[neighbour.k][neighbour.i][neighbour.j];
		if (n->_so_far < 0 || (n->_so_far > nei->_so_far + 1))
		{
			n->_from = neighbour;
			n->_so_far = nei->_so_far + 10;
			n->_priority = n->_so_far + manhatan(source, target);
		}
	}

	static void close(const cgra_node& node)
	{
		_nodes[node.k][node.i][node.j]._closed = 1;
	}

	static void write_path(const cgra_node& target, uint64_t (*cgra_conf)[3][4], uint8_t *from = nullptr)
	{
		const cgra_node n = _nodes[target.k][target.i][target.j]._from;

		// write node
		if (cgra_conf[target.k][target.i][target.j] == 0 || from)
		{
			char c;

			if (target.i > n.i)
				c = FunctionalUnit::UP;
			else if (target.i < n.i)
				c = FunctionalUnit::DOWN;
			else if (target.j > n.j)
				c = FunctionalUnit::LEFT;
			else if (target.j < n.j)
				c = FunctionalUnit::RIGHT;
			else
				c = 0;

			c += 64;

			if (from)
			{
				*from = c;
			}
			else
				cgra_conf[target.k][target.i][target.j] = (c << 25) + 0x7f;
		}

		// stop if start reached
		if (n == target)
			return;

		write_path(n, cgra_conf);
	}

	static bool is_open(const cgra_node& node)
	{
		return _nodes[node.k][node.i][node.j]._open;
	}

	static bool is_closed(const cgra_node& node)
	{
		return _nodes[node.k][node.i][node.j]._closed;
	}

	static int priority(const cgra_node& node)
	{
		return _nodes[node.k][node.i][node.j]._priority;
	}

};

astar_node (*astar_node::_nodes)[3][4] = nullptr;

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
bool route(const cgra_node& source, const cgra_node& target, uint64_t (*cgra_conf)[3][4], uint8_t * from)
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
			astar_node::write_path(target, cgra_conf, from);
			return true;
		}

		// don't visit node again
		open.pop();
		astar_node::close(current);

		// for each neighbour
		cgra_node a={0,1,1},b={1,0,1},c={0,-1,1},d={-1,0,1},e={0,0,1};
		for (cgra_node n : {a,b,c,d,e})
		{
			// position of neighbour
			cgra_node nn = n+current;

			if (nn.i >= 0 && nn.i < 3 && nn.j >= 0 && nn.j < 4 && nn.k < II)
			{
				// if neighbour already visited, do nothing
				if (astar_node::is_closed(nn) || (cgra_conf[nn.k][nn.i][nn.j] != 0 && !(nn == target)))
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
