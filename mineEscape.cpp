// Project Identifier: 19034C8F3B1196BF8E0C6E1C0F973D2FD550B88F

#include <vector>
#include <queue>
#include <iostream>
#include <getopt.h>
#include <set>
#include <iomanip>
#include <algorithm>
#include "P2random.h"

struct point{
	size_t x;
	size_t y;
};

class MineEscapeOptions {
public:
	MineEscapeOptions(int argc, char* argv[]);

	//getter functions
	int getStats() const;
	bool isMedian() const;
	bool isVerbose() const;

private:
	// Helper functions
	void printHelp() const;

	// List of commands
	int stats = -1; // Use -1 to indicate that stats is not provided
	bool median = false;
	bool verbose = false;
};

MineEscapeOptions::MineEscapeOptions(int argc, char* argv[]) {
	const char* short_opts = "hs:mv";
	const option long_opts[] = {
		{"help", no_argument, nullptr, 'h'},
		{"stats", required_argument, nullptr, 's'},
		{"median", no_argument, nullptr, 'm'},
		{"verbose", no_argument, nullptr, 'v'},
		{nullptr, 0, nullptr, 0}
	};

	int opt;
	while ((opt = getopt_long(argc, argv, short_opts, long_opts, nullptr)) != -1) {
		switch (opt) {
			case 'h':
				printHelp();
				exit(0);
				break;
			case 's':
				stats = std::atoi(optarg);
				break;
			case 'm':
				median = true;
				break;
			case 'v':
				verbose = true;
				break;
			default:
				printHelp();
				exit(1);
		}
	}

}

void MineEscapeOptions::printHelp() const {
	std::cout << "Usage: mineEscape [options]\n"
			  << "-h, --help\t\tPrint this help message and exit.\n"
			  << "-s, --stats N\t\tPrint extra summarization statistics with argument N.\n"
			  << "-m, --median\t\tPrint the median difficulty of clearing a rubble tile.\n"
			  << "-v, --verbose\t\tPrint out every rubble value as a tile is being cleared.\n";
}

int MineEscapeOptions::getStats() const {
	return stats;
}

bool MineEscapeOptions::isMedian() const {
	return median;
}

bool MineEscapeOptions::isVerbose() const {
	return verbose;
}

void printHelp() {
	std::cout << "Usage: mineEscape [options]\n"
			  << "-h, --help\t\tPrint this help message and exit.\n"
			  << "-s, --stats N\t\tPrint extra summarization statistics with argument N.\n"
			  << "-m, --median\t\tPrint the median difficulty of clearing a rubble tile.\n"
			  << "-v, --verbose\t\tPrint out every rubble value as a tile is being cleared.\n";
}

std::vector<std::vector<int>> getMap(std::istream &input, size_t size){
	std::vector<std::vector<int>> map;

	map.resize(size);

	for(auto &row : map)
		row.resize(size);

	for(size_t row = 0; row < size; ++row)
		for(size_t col = 0; col < size; ++col)
			input >> map[col][row];

	return map;

}

std::vector<std::vector<int>> read(point& start){

	std::string temp;
	std::cin >> temp;

	if(temp == "M"){
		std::cin >> temp;

		if(temp != "Size:")
			throw std::runtime_error("Invalid format.");
		uint32_t size;

		std::cin >> size;
		std::cin >> temp;

		if(temp != "Start:")
			throw std::runtime_error("Invalid format.");

		std::cin >> start.y;
		std::cin >> start.x;

		if (start.x >= size)
			throw std::runtime_error("Invalid starting column\n");

		if (start.y >= size)
			throw std::runtime_error("Invalid starting row\n");

		return getMap(std::cin, size);
	}

	else if(temp == "R"){
		
		std::cin >> temp;

		if(temp != "Size:")
			throw std::runtime_error("Invalid format.");

		uint32_t size;
		std::cin >> size;
		std::cin >> temp;

		if(temp != "Start:")
			throw std::runtime_error("Invalid format.");

		std::cin >> start.y;
		std::cin >> start.x;

		if(start.x >= size)
			throw std::runtime_error("Invalid starting column\n");

		if(start.y >= size)
			throw std::runtime_error("Invalid starting row\n");

		std::cin >> temp;
		if(temp != "Seed:")
			throw std::runtime_error("Invalid format.");

		uint32_t seed;
		std::cin >> seed;

		std::cin >> temp;
		if(temp != "Max_Rubble:")
			throw std::runtime_error("Invalid format.");

		uint32_t maxRubble;
		std::cin >> maxRubble;

		std::cin >> temp;
		if(temp != "TNT:")
			throw std::runtime_error("Invalid format.");
		
		uint32_t TNT;
		std::cin >> TNT;

		std::stringstream ss;
		P2random::PR_init(ss, size, seed, maxRubble, TNT);

		return getMap(ss, size);
	}
	else
		throw std::runtime_error("Invalid input mode\n");
}

struct medianHandler
{
	std::multiset<int> data;
	std::multiset<int>::iterator middle;

	void insert(int value)
	{
		double medianValue;

		auto inserted = data.insert(value);
		if (data.size() == 1)
		{
			middle = inserted;
			medianValue = value;
		}
		else
		{
			if (data.size() & 1)
			{
				if (value >= *middle)
					++middle;
				medianValue = *middle;
			}
			else
			{
				if (value < *middle)
					--middle;
				auto next = std::next(middle);
				medianValue = (*middle + *next) / 2.0;
			}
		}
		
		std::cout << std::fixed << std::setprecision(2);
		std::cout << "Median difficulty of clearing rubble is: " << medianValue << '\n';
	}
};

using pointValue = std::pair<point, int>;

bool operator==(const pointValue &a, const pointValue &b)
{
	return a.first.x == b.first.x && a.first.y == b.first.y && a.second == b.second;
}

//making a comparison operator for tiles
bool pointValueGreater(const pointValue &a, const pointValue &b) {
    if (a.second == -1 && b.second != -1) return false;
    if (b.second == -1 && a.second != -1) return true;

    if (a.second != b.second) return a.second > b.second;

    if (a.first.x != b.first.x) return a.first.x > b.first.x;

    return a.first.y > b.first.y;
};

constexpr int investigated = -2;
constexpr int cleared = -3;

void handleTNT(std::vector<std::vector<int>>& map, const pointValue& detonationPoint,
			   std::priority_queue<pointValue, std::vector<pointValue>, decltype(&pointValueGreater)>& discovered,
			   const MineEscapeOptions &format, int &total, size_t &numCleared, std::vector<pointValue> &stats, medianHandler &median) {
	constexpr int directions[4][2] = {{-1,0},{0,-1},{1,0},{0,1}};
	std::priority_queue<pointValue, std::vector<pointValue>, decltype(&pointValueGreater)> tntQueue(&pointValueGreater);
	tntQueue.push(detonationPoint);
	// clear the first tile
	map[detonationPoint.first.x][detonationPoint.first.y] = cleared;

	std::priority_queue<pointValue, std::vector<pointValue>, decltype(&pointValueGreater)> tntDiscovered(&pointValueGreater);

	while (!tntQueue.empty()) {
		pointValue curr = tntQueue.top();
		tntQueue.pop();

		if(format.isVerbose())
			std::cout << "TNT explosion at [" << curr.first.y << "," << curr.first.x << "]!\n";
		if (format.getStats() != -1)
			stats.push_back({{curr.first.x, curr.first.y}, -1});

		// explore adjacent tiles
		for (const auto& dir : directions) {
			int nextX = (int)curr.first.x + dir[0];
			int nextY = (int)curr.first.y + dir[1];

			// Check bounds.
			if (nextX >= 0 && (size_t)nextX < map.size() && nextY >= 0 && (size_t)nextY < map[0].size()) {
				// if the adjacent tile is TNT and not cleared, add it to the TNT queue.
				if (map[nextX][nextY] == -1) {
					tntQueue.push({{(size_t)nextX, (size_t)nextY}, -1});
					map[nextX][nextY] = cleared; // clear the tnt tile
				}
				else if (map[nextX][nextY] >= 0) {
					tntDiscovered.push({{(size_t)nextX, (size_t)nextY}, map[nextX][nextY]});
					
					map[nextX][nextY] = cleared; // set to cleared by tnt
				}
			}
		}
	}

	while (!tntDiscovered.empty()) {
		pointValue curr = tntDiscovered.top();
		tntDiscovered.pop();

		if (curr.second > 0)
		{
			if (format.isVerbose())
				std::cout << "Cleared by TNT: " << curr.second << " at [" << curr.first.y << "," << curr.first.x << "]\n";
			if(format.isMedian())
				median.insert(curr.second);
			if (format.getStats() != -1)
				stats.push_back(curr);
			
			total += curr.second;
			++numCleared;
		}
		
		curr.second = 0;
		discovered.push(curr);
	}
}

void pathFinder(std::vector<std::vector<int>>& map, const size_t startX, const size_t startY, const MineEscapeOptions &outputFormat){
	medianHandler median;
	std::vector<pointValue> stats;

	size_t mapHeight = map.size();
	size_t mapWidth = map[0].size();
	constexpr int directions[4][2] = {{-1,0},{0,-1},{1,0},{0,1}};

	int total = 0;
	size_t numCleared = 0;

	//one for discovered tiles, one for tnt tiles
	std::priority_queue<pointValue, std::vector<pointValue>, decltype(&pointValueGreater)> discovered(&pointValueGreater);

	discovered.push({{startX, startY}, map[startX][startY]});

	while(!discovered.empty()){
		pointValue curr = discovered.top();
		discovered.pop();

		if (map[curr.first.x][curr.first.y] == investigated)
			continue;
		
		if (map[curr.first.x][curr.first.y] != cleared)
		{
			if(curr.second > 0) {
				total += curr.second;

				if(outputFormat.isVerbose())
					std::cout << "Cleared: " << curr.second << " at [" << curr.first.y << "," << curr.first.x << "]\n";
				if(outputFormat.isMedian())
					median.insert(curr.second);
				if (outputFormat.getStats() != -1)
					stats.push_back(curr);
				
				++numCleared;
			}
			else if (curr.second == -1) {
				handleTNT(map, curr, discovered, outputFormat, total, numCleared, stats, median);
				continue;
			}
		}

		if (curr.first.x == mapWidth - 1 || curr.first.y == mapHeight - 1 || curr.first.x == 0 || curr.first.y == 0)
			break;

		map[curr.first.x][curr.first.y] = investigated;

		for (const auto &dir : directions) {
			int nextX = (int)curr.first.x + dir[0];
			int nextY = (int)curr.first.y + dir[1];

			//check if the tile is in bound
			if (nextX >= 0 && (size_t)nextX < mapWidth && nextY >= 0 && (size_t)nextY < mapHeight && map[nextX][nextY] != cleared && map[nextX][nextY] != investigated)
				discovered.push({{(size_t)nextX, (size_t)nextY}, map[nextX][nextY]});
		}
	}

	std::cout << "Cleared " << numCleared << " tiles containing " << total << " rubble and escaped.\n";

	if(outputFormat.getStats() != -1){

		std::cout << "First tiles cleared:\n";

		for(int i = 0; i < outputFormat.getStats() && (size_t)i < stats.size(); ++i)
			if(stats[i].second == -1)
				std::cout << "TNT at [" << stats[i].first.y << "," << stats[i].first.x << "]\n";
			else
				std::cout << stats[i].second << " at [" << stats[i].first.y << "," << stats[i].first.x << "]\n";

		std::cout << "Last tiles cleared:\n";

		for(int i = 1; i <= outputFormat.getStats() && (size_t)i <= stats.size(); ++i)
			if(stats[stats.size() - i].second == -1)
				std::cout << "TNT at [" << stats[stats.size() - i].first.y << "," << stats[stats.size() - i].first.x << "]\n";
			else
				std::cout << stats[stats.size() - i].second << " at [" << stats[stats.size() - i].first.y  << "," << stats[stats.size() - i].first.x << "]\n";

		std::sort(stats.begin(), stats.end(), pointValueGreater);

		std::cout << "Easiest tiles cleared:\n";

		for(int i = 1; i <= outputFormat.getStats() && (size_t)i <= stats.size(); ++i)
			if(stats[stats.size() - i].second == -1)
				std::cout << "TNT at [" << stats[stats.size() - i].first.y << "," << stats[stats.size() - i].first.x << "]\n";
			else
				std::cout << stats[stats.size() - i].second << " at [" << stats[stats.size() - i].first.y << "," << stats[stats.size() - i].first.x << "]\n";

		std::cout << "Hardest tiles cleared:\n";

		for(int i = 0; i < outputFormat.getStats() && (size_t)i < stats.size(); ++i)
			if(stats[i].second == -1)
				std::cout << "TNT at [" << stats[i].first.y << "," << stats[i].first.x << "]\n";
			else
				std::cout << stats[i].second << " at [" << stats[i].first.y << "," << stats[i].first.x << "]\n";
	}
}

int main(int argc, char *argv[]) {

	MineEscapeOptions options(argc, argv);
	point start;
	std::vector<std::vector<int>> map;
	try
	{
		map = read(start);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	pathFinder(map, start.x, start.y, options);

	return 0;
}
