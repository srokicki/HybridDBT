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
#include <stack>

#define LOG_LO 0
#define LOG_HI 0

int II;
int currentII;

using namespace cgra;

class astar_node_less
{
public:
	bool operator()(const cgra_node& l, const cgra_node& r);
};

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
	static uint8_t _opcode;
	static uint64_t	 (*_cgra_conf)[CgraSimulator::height][CgraSimulator::width];
public:
	static void init(const cgra_node& dep1, const cgra_node& dep2, uint64_t (*cgra_conf)[CgraSimulator::height][CgraSimulator::width], uint8_t opcode)
	{
		_dep1 = dep1;
		_dep2 = dep2;
		_cgra_conf = cgra_conf;
		_opcode = opcode;

		//Log::out(0) << opcodeNames[opcode] << " has " << (_dep1.i != -1)+(_dep2.i != -1) << " dependencies.\n";
	}

	bool operator() (const cgra_node& a, const cgra_node& b)
	{

		if (!(_opcode == VEX_STB || _opcode == VEX_STD || _opcode == VEX_STH || _opcode == VEX_STW ||
				_opcode == VEX_LDB || _opcode == VEX_LDD || _opcode == VEX_LDH || _opcode == VEX_LDW))
			if (a.i == 0 && a.j == 1)
				return 1;
			else if (b.i == 0 && b.j == 1)
				return 0;

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

				if (na.i >= 0 && na.i < CgraSimulator::height && na.j >= 0 && na.j < CgraSimulator::width && _cgra_conf[na.k][na.i][na.j] == 0) ++freedoma;
				if (nb.i >= 0 && nb.i < CgraSimulator::height && nb.j >= 0 && nb.j < CgraSimulator::width && _cgra_conf[nb.k][nb.i][nb.j] == 0) ++freedomb;
			}
			return freedoma < freedomb;
		}
		else
			return da > db;
	}
};

cgra_node dist_from_deps::_dep1;
cgra_node dist_from_deps::_dep2;
uint8_t dist_from_deps::_opcode;
uint64_t (*dist_from_deps::_cgra_conf)[CgraSimulator::height][CgraSimulator::width];

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
void setupSources(const uint128_struct * instructions, source * sources, int16_t *global, uint32_t numInstructions);

bool route(const cgra_node& source, const cgra_node& target, uint64_t (*cgra_conf)[CgraSimulator::height][CgraSimulator::width], uint8_t *from);

int findII(uint128_struct * instructions, source * sources, uint32_t numInstructions)
{
	int16_t * depth = new int16_t[numInstructions];
	int16_t ret = 1;
	int16_t nbMem = 0;

	for (uint32_t i = 0; i < numInstructions; ++i)
	{
		uint8_t opcode = ((instructions[i].word96) >> 19) & 0x7f;
		if (opcode == VEX_STB || opcode == VEX_STH || opcode == VEX_STW || opcode == VEX_STD
				|| opcode == VEX_LDB || opcode == VEX_LDH || opcode == VEX_LDW || opcode == VEX_LDD)
			nbMem++;

		depth[i] = 1;
		if (sources[i].src1 != -1)
			depth[i] = depth[sources[i].src1] + 1;
		if (sources[i].src2 != -1)
			depth[i] = std::max((int)depth[i], depth[sources[i].src2] + 1);


		ret = std::max(ret, depth[i]);
	}

	return std::max(ret, nbMem);
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
		ret = ret && (u.features() & FunctionalUnit::FEATURE_MEM);
		if (virtualRDest >= 256)
		{
			ret = ret && (u.features() & FunctionalUnit::FEATURE_REG);
		}
	}
	else
	{
		if (virtualRIn2 >= 256)
		{
			ret = ret && (u.features() & FunctionalUnit::FEATURE_REG);
		}

		if (!isImm && virtualRIn1_imm9 >= 256)
		{
			ret = ret && (u.features() & FunctionalUnit::FEATURE_REG);
		}

		if (virtualRDest >= 256)
		{
			ret = ret && (u.features() & FunctionalUnit::FEATURE_REG);
		}
	}
	return ret;
}

void printTikz(uint64_t (* configuration)[CgraSimulator::height][CgraSimulator::width], unsigned int ii)
{
	static const std::string tabs[6] = { "black", "yellow", "red", "blue", "green", "orange" };
	std::cout << "\\begin{tikzpicture}\n";

	for (unsigned int depth = 0; depth < ii; ++depth)
	{

		std::cout
				<< "\\begin{scope}\n"
				<< "\\pgftransformcm{1}{0}{0.4}{0.5}{\\pgfpoint{0cm}{"<<0-(int)(depth*CgraSimulator::width)<<"cm}}\n";
		for (unsigned int i = 0; i < CgraSimulator::height; ++i)
		{

			for (unsigned int j = 0; j < CgraSimulator::width; ++j)
			{
				auto instr = configuration[depth][i][j];
				auto opcode = cgra::opcode(instr);
				auto ra = cgra::regA(instr);
				auto rb = cgra::regB(instr);
				auto rc = cgra::regC(instr);

				bool isImm = (((opcode >> 4) & 0x7) == 2);
				unsigned int id = i*CgraSimulator::width+j;
				std::cout << "\\node (n"<< depth << i << j <<") at (" << i*2 << "," << j*2 << ") [circle,fill="<<(instr ? "gray" : tabs[depth])<<"] {};\n";
				if (ra > 63)
				{
					std::cout << "\\draw[black,very thick] (n" << depth << i << j << ") -- (n" << depth-1;
					switch (ra-64)
					{
					case 0: std::cout << i << j; break;
					case 1: std::cout << i << j-1; break;
					case 2: std::cout << i << j+1; break;
					case 3: std::cout << i-1 << j; break;
					case 4: std::cout << i+1 << j; break;
					default:
						std::cout << "ERROR DRAWING TIKZ\n";
						exit(-1);
					}
					std::cout << ");\n";
				}

				if (!isImm && rc > 63)
				{
					std::cout << "\\draw[black,very thick] (n" << depth << i << j << ") -- (n" << depth-1;
					switch (rc-64)
					{
					case 0: std::cout << i << j; break;
					case 1: std::cout << i << j-1; break;
					case 2: std::cout << i << j+1; break;
					case 3: std::cout << i-1 << j; break;
					case 4: std::cout << i+1 << j; break;
					default:
						std::cout << "ERROR DRAWING TIKZ\n";
						exit(-1);
					}
					std::cout << ");\n";
				}
				if ((opcode == VEX_STB || opcode == VEX_STD || opcode == VEX_STH || opcode == VEX_STW) && rb > 63)
				{
					std::cout << "\\draw[black,very thick] (n" << depth << i << j << ") -- (n" << depth-1;
					switch (rb-64)
					{
					case 0: std::cout << i << j; break;
					case 1: std::cout << i << j-1; break;
					case 2: std::cout << i << j+1; break;
					case 3: std::cout << i-1 << j; break;
					case 4: std::cout << i+1 << j; break;
					default:
						std::cout << "ERROR DRAWING TIKZ\n";
						exit(-1);
					}
					std::cout << ");\n";
				}
			}

		}
		std::cout << "\\end{scope}\n";
	}
	std::cout << "\\end{tikzpicture}\n";
}

NodeCentricScheduler::NodeCentricScheduler()
{

}

bool NodeCentricScheduler::schedule(VexCgraSimulator& cgra, uint128_struct * instructions, uint32_t numInstructions)
{
	const FunctionalUnit * units = cgra.cgraSimulator.units();

	//Log::out(2) << "Hello\n";
	//printGraph(instructions, numInstructions);

	source * sources = new source[numInstructions];
	int16_t * global = new int16_t[numInstructions];
	cgra_node * placeOfInstr = (new cgra_node[numInstructions+1])+1;

	uint8_t lastRead[64] = {0};

	placeOfInstr[-1] = {-1,-1,-1};

	setupSources(instructions, sources, global, numInstructions);
	II = findII(instructions, sources, numInstructions);

	currentII = II;
	while (currentII != II*2)
	{
		// setup the scheduling space
		uint64_t (*configuration)[CgraSimulator::height][CgraSimulator::width] = new uint64_t[currentII][CgraSimulator::height][CgraSimulator::width];
		uint64_t (*new_conf)[CgraSimulator::height][CgraSimulator::width] = new uint64_t[currentII][CgraSimulator::height][CgraSimulator::width];
		for (int i = 0; i < currentII; ++i)
			for (int j = 0; j < CgraSimulator::height; ++j)
				for (int k = 0; k < CgraSimulator::width; ++k)
					configuration[i][j][k] = 0;

		bool routed = true;

		// for each instruction to schedule
		for (uint32_t instrId = 0; instrId < numInstructions; ++instrId)
		{
			if (currentII == II)
			{
				//Log::out(2) << printBytecodeInstruction(0, instructions[instrId].word96, instructions[instrId].word64, instructions[instrId].word32, instructions[instrId].word0);
			}
			std::priority_queue<cgra_node, std::vector<cgra_node>, dist_from_deps> possible;
			uint128_struct instruction = instructions[instrId];
			uint8_t src1 = 0xff, src2 = 0xff;

			routed = false; cgra_node place;

			cgra_node n1 = placeOfInstr[sources[instrId].src1], n2 = placeOfInstr[sources[instrId].src2];

			dist_from_deps::init(
						n1
						, n2
						, configuration
						, ((instruction.word96) >> 19) & 0xff);

			static const cgra_node errNode = {-1,-1,-1};

			uint8_t nameDependency = 0;
			if (global[instrId] != -1)
			{
				nameDependency = lastRead[global[instrId]];
			}

			// compute all available places, sorted by priority given
			// by [dist_from_deps] class
			for (int k = std::max(std::max(n1.k, n2.k)+1, (int)nameDependency); k < currentII; ++k)
			{
				for (int i = 0; i < CgraSimulator::height; ++i)
				{
					for (int j = 0; j < CgraSimulator::width; ++j)
					{
						int kk = 0;
						if (!(n1 == errNode))
						{
							kk = manhatan({i,j,n1.k},n1);
							if (!(n2 == errNode))
								kk = std::max(kk, manhatan({i,j,n2.k},n2));
						}

						if (k >= kk && !configuration[k][i][j] && canPlace(units[i*CgraSimulator::width+j], instruction))
						{
							//Log::out(0) << i << "," << j << "," << k << "\n";
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

				std::memcpy(new_conf, configuration, currentII*CgraSimulator::height*CgraSimulator::width*sizeof(uint64_t));
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
				std::memcpy(configuration, new_conf, II*CgraSimulator::height*CgraSimulator::width*sizeof(uint64_t));
				uint16_t read1 = 64, read2 = 64;
				configuration[place.k][place.i][place.j] = cgra::vex2cgra(instruction, src1, src2, &read1, &read2);
				if (read1 != 64)
				{
					lastRead[read1] = place.k;
				}
				if (read2 != 64)
				{
					lastRead[read2] = place.k;
				}
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

			instructions[0] = {VEX_CGRA << 19,id << 23,0,0};

//			for (int i = 0; i < currentII; ++i)
//			{
//				Log::out(0) << "LAYER " << i << "\n";
//				cgra::printConfig(0, (uint64_t*)(configuration[i]));
//			}

			//printTikz(configuration, currentII);
			return true;
		}

		// if we haven't managed to schedule with this II, increase it and retry
		delete[] new_conf;
		delete[] configuration;
		currentII++;
	}

	// if we can't finish the schedule even with a big enough II
	// it is because some path is blocked. We don't handle this case.
	//Log::out(0) << "CANNOT SCHEDULE with II = " << II*2 << "\n";
	return false;
}

void setupSources(const uint128_struct * instructions, source * sources, int16_t * global, uint32_t numInstructions)
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

		global[instrId] = virtualRDest;
		if ((opCode == VEX_STB || opCode == VEX_STD || opCode == VEX_STH || opCode == VEX_STW))
		{
			if (virtualRIn2 < 256)
			{
				sources[instrId].src1 = virtualRIn2;
				global[virtualRIn2] = -1;
			}

			if (virtualRDest < 256)
			{
				sources[instrId].src2 = virtualRDest;
				global[virtualRDest] = -1;
			}
		}
		else
		{
			if (virtualRIn2 < 256)
			{
				sources[instrId].src1 = virtualRIn2;
				global[virtualRIn2] = -1;
			}

			if (!isImm && virtualRIn1_imm9 < 256)
			{
				sources[instrId].src2 = virtualRIn1_imm9;
				global[virtualRIn1_imm9] = -1;
			}
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

int16_t (*carry_value)[CgraSimulator::height][CgraSimulator::width];
int16_t src1, src2;

uint64_t (*conf)[CgraSimulator::height][CgraSimulator::width];
uint64_t (*new_conf)[CgraSimulator::height][CgraSimulator::width];
cgra_node (*take_from)[CgraSimulator::height][CgraSimulator::width];

class astar_node
{
	static astar_node (*_nodes)[CgraSimulator::height][CgraSimulator::width];
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

		_nodes = new astar_node[currentII][CgraSimulator::height][CgraSimulator::width];
		for (int k = 0; k < currentII; ++k)
		{
			for (int i = 0; i < CgraSimulator::height; ++i)
			{
				for (int j = 0; j < CgraSimulator::width; ++j)
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
		astar_node * n = &_nodes[source.k % currentII][source.i][source.j];
		n->_open = open;
		n->_closed = closed;
		n->_from = from;
		n->_so_far = so_far;
		n->_priority = so_far + manhatan(source, target);
	}

	static void open(const cgra_node& source, const cgra_node& neighbour, const cgra_node& target)
	{
		astar_node * n = &_nodes[source.k % currentII][source.i][source.j];
		n->_open = 1;
	}

	static void update(const cgra_node& source, const cgra_node& neighbour, const cgra_node& target)
	{
		astar_node * n = &_nodes[source.k % currentII][source.i][source.j], * nei = &_nodes[neighbour.k % currentII][neighbour.i][neighbour.j];
		if (n->_so_far < 0 || (n->_so_far > nei->_so_far + 1))
		{
			n->_from = neighbour;
			n->_so_far = nei->_so_far + 10;
			n->_priority = n->_so_far + manhatan(source, target);
		}
	}

	static void close(const cgra_node& node)
	{
		_nodes[node.k % currentII][node.i][node.j]._closed = 1;
	}

	static void write_path(const cgra_node& target, uint64_t (*cgra_conf)[CgraSimulator::height][CgraSimulator::width], uint8_t *from = nullptr)
	{
		const cgra_node n = _nodes[target.k % currentII][target.i][target.j]._from;

		// write node
		if (cgra_conf[target.k % currentII][target.i][target.j] == 0 || from)
		{
			uint8_t c;

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
			{
				carry_value[target.k % currentII][target.i][target.j] = src2;
				cgra_conf[target.k % currentII][target.i][target.j] = (((uint64_t)c) << CGRA_REG1_OFFSET) + CGRA_CARRY;
			}
		}

		// stop if start reached
		if (n == target)
			return;

		write_path(n, cgra_conf);
	}

	static bool is_open(const cgra_node& node)
	{
		return _nodes[node.k % currentII][node.i][node.j]._open;
	}

	static bool is_closed(const cgra_node& node)
	{
		return _nodes[node.k % currentII][node.i][node.j]._closed;
	}

	static int priority(const cgra_node& node)
	{
		return _nodes[node.k % currentII][node.i][node.j]._priority;
	}

};

astar_node (*astar_node::_nodes)[CgraSimulator::height][CgraSimulator::width] = nullptr;

bool astar_node_less::operator()(const cgra_node& l, const cgra_node& r)
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
bool route(const cgra_node& source, const cgra_node& target, uint64_t (*cgra_conf)[CgraSimulator::height][CgraSimulator::width], uint8_t * from)
{
	// initialization of nodes+priority queue
	std::priority_queue<cgra_node, std::vector<cgra_node>, astar_node_less > open;
	astar_node::init();

	// open = { start }
	open.push(source);
	astar_node::init_node(source, 1, 0, source, 0, target);

	while (!open.empty())
	{
		cgra_node current = open.top();

		// target reached, configure path
		if (current == target || carry_value[current.k][current.i][current.j] == src2)
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

			if (nn.i >= 0 && nn.i < CgraSimulator::height && nn.j >= 0 && nn.j < CgraSimulator::width&& nn.k < currentII*2)
			{
				// if neighbour already visited, or not available, do nothing
				if (astar_node::is_closed(nn)
						|| (cgra_conf[nn.k % currentII][nn.i][nn.j] != 0
								&& !(nn == target || carry_value[nn.k % currentII][nn.i][nn.j] == src2)))
				{
					continue;
				}

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

void setupNbDeps(source * sources, uint8_t * nbDeps, uint32_t numSources)
{
	for (uint32_t sourceId = 0; sourceId < numSources; ++sourceId)
	{
		nbDeps[sourceId] = (sources[sourceId].src1 != -1) + (sources[sourceId].src2 != -1);
	}
}

EdgeCentricScheduler::EdgeCentricScheduler()
{
}

bool is_special(uint128_struct instr)
{
	return canPlace(FunctionalUnit(), instr);
}

class possible_or_not
{
	static VexCgraSimulator * _cgra;
	static uint128_struct _instr;
public:
	static void init(VexCgraSimulator& cgra, uint128_struct instr)
	{
		_cgra = &cgra;
		_instr = instr;
	}

	bool operator()(const cgra_node& l, const cgra_node& r)
	{
		if (is_special(_instr))
			return canPlace(_cgra->cgraSimulator.units()[l.i*CgraSimulator::width+l.j], _instr);
		else
			return !(_cgra->cgraSimulator.units()[l.i*CgraSimulator::width+l.j].features() & (FunctionalUnit::FEATURE_MEM));
	}
};

VexCgraSimulator (*possible_or_not::_cgra);
uint128_struct possible_or_not::_instr;

constexpr cgra_node noPlace = {-1,-1,-1};

bool EdgeCentricScheduler::schedule(VexCgraSimulator &cgra, uint128_struct *instructions, uint32_t numInstructions)
{
	source * sources = new source[numInstructions];
	int16_t * globals = new int16_t[numInstructions];
	uint8_t * nbDeps = new uint8_t[numInstructions];
	cgra_node * placeOfInstr = new cgra_node[numInstructions];


	uint32_t lastRead[64] = {0};

	setupSources(instructions, sources, globals, numInstructions);
	setupNbDeps(sources, nbDeps, numInstructions);

	currentII = II = findII(instructions, sources, numInstructions);

	conf = nullptr;
	new_conf = nullptr;
	take_from = nullptr;
	carry_value = nullptr;

	bool routed;

	unsigned int theId = 0;
	while (currentII < II*2)
	{
		// initialize configuration space
		uint64_t (*tmp)[CgraSimulator::height][CgraSimulator::width] = new uint64_t[currentII][CgraSimulator::height][CgraSimulator::width];
		int16_t (*carry_tmp)[CgraSimulator::height][CgraSimulator::width] = new int16_t[currentII][CgraSimulator::height][CgraSimulator::width];

		std::memset(tmp, 0, currentII*CgraSimulator::height*CgraSimulator::width*sizeof(uint64_t));
		std::memset(carry_tmp, -1, currentII*CgraSimulator::height*CgraSimulator::width*sizeof(int16_t));
		if (conf)
		{
			std::memcpy(tmp, conf, (currentII-1)*CgraSimulator::height*CgraSimulator::width*sizeof(uint64_t));
			delete[] conf;
		}

		if (carry_value)
		{
			std::memcpy(carry_tmp, carry_value, (currentII-1)*CgraSimulator::height*CgraSimulator::width*sizeof(int16_t));
			delete[] carry_value;
		}

		carry_value = carry_tmp;
		conf = tmp;		
		if (new_conf)
			delete[] new_conf;
		new_conf = new uint64_t[currentII][CgraSimulator::height][CgraSimulator::width];
		std::memset(new_conf, 0, currentII*CgraSimulator::height*CgraSimulator::width*sizeof(uint64_t));

		if (take_from)
			delete[] take_from;
		take_from = new cgra_node[currentII][CgraSimulator::height][CgraSimulator::width];
		std::memset(take_from, 0, currentII*CgraSimulator::height*CgraSimulator::width*sizeof(cgra_node));

		//Log::out(0) << "New scheduling space with II=" << currentII << "\n";
		for (unsigned int i = 0; i < currentII; ++i)
		{
			//Log::out(0) << "LAYER " << i << "\n";
			//printConfig(0, (uint64_t*)conf[i]);
			for (unsigned int j = 0; j < CgraSimulator::height; ++j)
			{
				for (unsigned int k = 0; k < CgraSimulator::width; ++k)
				{
					//Log::out(0) << carry_value[i][j][k] << "         ";
				}
				//Log::out(0) << "\n";
			}
		}


		// while there is an instruction to place
		for (; theId < numInstructions; ++theId)
		{
			// chose instruction
			uint128_struct theInstr = instructions[theId];
			uint8_t from = 0xff;
			cgra_node place;

			possible_or_not::init(cgra, theInstr);

			bool first_is_first;
			routed = false;
			//Log::out(0) << "\nPlacing instruction " << theId << ": " << printBytecodeInstruction(theId, theInstr.word96,
			//																																									 theInstr.word64, theInstr.word32,
			//																																									 theInstr.word0);


			// setup [src1] and [src2], [src1] will always be valid if [src2] is valid
			// it allows a simpler control flow when generating the paths
			if (sources[theId].src1 != -1)
			{
				src1 = sources[theId].src1;
				src2 = sources[theId].src2;
				first_is_first = true;
			}
			else
			{
				src1 = sources[theId].src2;
				src2 = -1;
				first_is_first = false;
			}

			// place instruction
			// start from a source
			if (src1 != -1)
			{
				for (unsigned int k = 0; k < currentII; ++k)
					for (unsigned int i = 0; i < CgraSimulator::height; ++i)
						for (unsigned int j = 0; j < CgraSimulator::width; ++j)
							take_from[k][i][j] = noPlace;

				routed = false;

				// browse all possible routes from the source
				std::priority_queue<cgra_node, std::vector<cgra_node>, possible_or_not> possible;
				cgra_node thePlace = placeOfInstr[src1];
				//Log::out(0) << "\tThe instruction has a [src1] = " << src1 << " at (" << thePlace.i << ", " << thePlace.j << ", " << thePlace.k << ")\n";

				// setup first-step possible paths
				cgra_node up={-1,0,1},down={1,0,1},left={0,-1,1},right={0,1,1},center={0,0,1};
				for (cgra_node displacement : {up,down,left,right,center})
				{
					cgra_node path = thePlace + displacement;
					if (path.i >= 0 && path.i < CgraSimulator::height && path.j >= 0 && path.j < CgraSimulator::width && path.k < currentII * 2
							&& (conf[path.k % currentII][path.i][path.j] == 0 || carry_value[path.k % currentII][path.i][path.j] == src1))
					{
						possible.push(path);
						take_from[path.k % currentII][path.i][path.j] = thePlace;
						conf[path.k % currentII][path.i][path.j] = 0xff;
					}
				}

				//Log::out(0) << "\tStarting to search in " << possible.size() << " nodes\n";
				// for each possible places
				while (!possible.empty())
				{
					// we will work on [new_conf] to schedule the instruction
					std::memcpy(new_conf, conf, currentII*CgraSimulator::height*CgraSimulator::width*sizeof(uint64_t));

					place = possible.top();

					//Log::out(0) << "\tTrying place (" << place.i << ", " << place.j << ", " << place.k << ")...\n";
					possible.pop();

					if (!canPlace(cgra.cgraSimulator.units()[place.i*CgraSimulator::width+place.j],theInstr))
					{
						//Log::out(0) << "\t\tPlace (" << place.i << ", " << place.j << ", " << place.k << ") is not valid.\n";
					}
					else
					{

						routed = true;
						// for each path, try to route other operands, if any
						if (src2 != -1)
						{
							//Log::out(0) << "\t\tTrying to route from [src2] to (" << place.i << ", " << place.j << ", " << place.k << ").\n";
							// reserve current [src1] path
							cgra_node backward = take_from[place.k % currentII][place.i][place.j];
							while (backward != placeOfInstr[src1])
							{
								new_conf[backward.k % currentII][backward.i][backward.j] = cgra::direction(take_from[backward.k % currentII][backward.i][backward.j]
										, backward);
								backward = take_from[backward.k % currentII][backward.i][backward.j];
							}

							// try to route [src2]
							routed = route(placeOfInstr[sources[theId].src2], place, new_conf, &from);
						}
					}

					// if we managed to route the instruction, we can place it
					if (routed)
						break;

					// else, we add new possible places, and continue browsing them
					cgra_node up={-1,0,1},down={1,0,1},left={0,-1,1},right={0,1,1},center={0,0,1};
					for (cgra_node displacement : {up,down,left,right,center})
					{
						cgra_node path = place + displacement;
						if (path.i >= 0 && path.i < CgraSimulator::height && path.j >= 0 && path.j < CgraSimulator::width && path.k < currentII * 2
								&& (new_conf[path.k % currentII][path.i][path.j] == 0 || carry_value[path.k % currentII][path.i][path.j] == src1) && (take_from[path.k % currentII][path.i][path.j] == noPlace))
						{
							possible.push(path);
							take_from[path.k % currentII][path.i][path.j] = place;
							if (conf[path.k % currentII][path.i][path.j] == 0)
								conf[path.k % currentII][path.i][path.j] = 0xff;
						}
					}
				} // while (!possible.empty())

			}
			else // if (sources[theId].src1 != -1)
			{
				uint8_t nameDependency = 0;
				if (globals[theId] != -1)
				{
					nameDependency = lastRead[globals[theId]];
				}

				routed = false;
				for (unsigned int k = nameDependency; k < currentII*2 && !routed; ++k)
				{
					for (unsigned int i = 0; i < CgraSimulator::height && !routed; ++i)
					{
						for (unsigned int j = 0; j < CgraSimulator::width && !routed; ++j)
						{
							if (conf[k%currentII][i][j] == 0 && canPlace(cgra.cgraSimulator.units()[i*CgraSimulator::width+j],instructions[theId]))
							{
								place = {i,j,k};
								routed = true;
							}
						}
					}
				}
			}

			for (unsigned int k = 0; k < currentII; ++k)
				for (unsigned int i = 0; i < CgraSimulator::height; ++i)
					for (unsigned int j = 0; j < CgraSimulator::width; ++j)
						if (new_conf[k][i][j] == 0xff)
							new_conf[k][i][j] = conf[k][i][j] = 0;

			if (routed)
			{
				uint8_t r1 = 0xff, r2 = 0xff;
				placeOfInstr[theId] = place;
				// construct path1
				if (src1 != -1)
				{
					if (first_is_first)
						r1 = 64 + cgra::direction(take_from[place.k % currentII][place.i][place.j], place);
					else
						r2 = 64 + cgra::direction(take_from[place.k % currentII][place.i][place.j], place);

					cgra_node p = take_from[place.k % currentII][place.i][place.j];
					while (p != placeOfInstr[src1])
					{
						carry_value[p.k % currentII][p.i][p.j] = src1;
						new_conf[p.k % currentII][p.i][p.j] = (((uint64_t)(64 + cgra::direction(take_from[p.k % currentII][p.i][p.j], p))) << CGRA_REG1_OFFSET) + CGRA_CARRY;
						p = take_from[p.k % currentII][p.i][p.j];
					}
				}

				// if we are here, path2 is already constructed by route(), we just need to get r2
				if (src2 != -1)
				{
					if (first_is_first)
						r2 = from;
					else
						r1 = from;
				}

				uint16_t read1 = 64, read2 = 64;
				new_conf[place.k % currentII][place.i][place.j] = cgra::vex2cgra(theInstr, r1, r2, &read1, &read2);
				if (read1 != 64)
				{
					lastRead[read1] = place.k;
				}
				if (read2 != 64)
				{
					lastRead[read2] = place.k;
				}

				std::memcpy(conf, new_conf, currentII*CgraSimulator::height*CgraSimulator::width*sizeof(uint64_t));

				for (unsigned int i = 0; i < currentII; ++i)
				{
					//Log::out(0) << "LAYER " << i << "\n";
					//printConfig(0, (uint64_t*)(conf[i]));
				}
			}
			else
				break;

		} // for (theId)

		if (routed)
			break;

		//Log::out(0) << "Couldn't schedule with II=" << currentII << " incrementing ...\n";
		currentII++;
	} // while (currentII < II*2)

	delete[] new_conf;
	delete[] take_from;
	if (routed)
	{
		// if properly configured, commit configuration
		auto id = cgra.configurationCache.size();
		cgra.configurationCache[id].cycles = currentII;
		cgra.configurationCache[id].configuration = (uint64_t*)conf;

		instructions[0] = {VEX_CGRA << 19,id << 23,0,0};
		//Log::out(0) << "Block scheduled\n";
		for (unsigned int i = 0; i < currentII; ++i)
						{
							//Log::out(0) << "LAYER " << i << "\n";
							//printConfig(0, (uint64_t*)(conf[i]));
						}
		return true;
	}
	else
	{
		Log::out(0) << "Failed to schedule block\n";
		delete[] conf;
		return false;
	}
}
