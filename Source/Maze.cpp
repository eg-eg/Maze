#include "Maze.h"

#include <random>
#include <time.h>

Maze::Maze()
{
}

Maze::Maze(const std::string path)
{
	load(path);
}

Node Maze::getStart() const
{
	Node tmp;

	//Loop through every node to ensure that there is only one
	//Return an invalid node if multiple are found to trigger error
	for (int i = 0; i < _maze.size(); i++)
	{
		for (int o = 0; o < _maze[i].size(); o++)
		{
			if (_maze[i][o].type == Node::Type::START)
			{
				if(tmp.steps == -1) tmp = _maze[i][o];
				else return Node();
			}
		}
	}

	return tmp;
}

Node Maze::getTarget() const
{
	Node tmp;

	//Loop through every node to ensure that there is only one
	//Return an invalid node if multiple are found to trigger error
	for (int i = 0; i < _maze.size(); i++)
	{
		for (int o = 0; o < _maze[i].size(); o++)
		{
			if (_maze[i][o].type == Node::Type::TARGET)
			{
				if (tmp.x == -1) tmp = _maze[i][o];
				else return Node();
			}
		}
	}

	return tmp;
}

void Maze::load(const std::string &path)
{
	_maze.clear();

	std::ifstream input;
	input.open(path);
	if (!input.is_open())
	{
		std::cout << "Could not open maze at: " << path << std::endl;
		return;
	}

	int rowCount = 0;
	std::string row;

	//Because files are read by row and not by column, the maze must have its x and y swapped
	//First it is loaded into a vector of strings, and then it is flipped after it is determined to be rectangular
	std::vector<std::string> tmpVector;
	while (std::getline(input, row))
	{
		tmpVector.push_back(row);
	}

	//Ensure that the maze is rectangular
	//This is done before other error checking to prevent crashes when rotating
	int xSize = tmpVector[0].size();
	for (int i = 1; i < tmpVector.size(); i++)
	{
		if (xSize != tmpVector[i].size())
		{
			std::cout << "Maze loaded from " << path << " is not a rectangle. Please check the maze and try again." << std::endl;
			return;
		}
	}

	_maze.resize(tmpVector[0].size());
	for (int i = 0; i < tmpVector[0].size(); i++)
	{
		_maze[i].resize(tmpVector.size());
	}

	//Convert the maze into nodes
	for (int i = 0; i < _maze.size(); i++)
	{
		for (int o = 0; o < _maze[0].size(); o++)
		{
			//Swap i and o for tmpVector to flip the maze
			_maze[i][o] = Node(Node::Type(tmpVector[o][i]), i, o);
		}
	}

	Node test = getStart();
	if (test.x == -1)
	{
		std::cout << "Maze loaded from " << path << " has no marked start or has multiple starts. Please check the maze and try again." << std::endl;
		return;
	}

	test = getTarget();
	if (test.x == -1)
	{
		std::cout << "Maze loaded from " << path << "has no marked end or multiple ends. Please check the maze and try again." << std::endl;
		return;
	}

	//Check if there is a top border
	bool border = true;
	for (int i = 0; i < _maze.size(); i++)
	{
		if (_maze[i][0].type != Node::Type::WALL)
		{
			border = false;
			break;
		}
	}

	//TODO: Add borders if they don't exist
}

bool Maze::stepProcess()
{
	//Call the other function so that the static vector is consistent
	return stepProcess(glm::vec2());
}

bool Maze::stepTrace()
{
	//Call the other function so that the static Node is consistent
	return stepTrace(glm::vec2());
}

bool Maze::stepProcess(glm::vec2 &updatePos)
{
	static std::vector<Node> unsearched;
	if (unsearched.size() == 0)
	{
		//Make sure start hasn't already been used
		Node tmp = getStart();
		if (tmp.steps == -1)
		{
			_maze[tmp.x][tmp.y].steps = 0;
			unsearched.push_back(_maze[tmp.x][tmp.y]);
		}
		else return false;
	}

	Node currentNode = unsearched[0];
	unsearched.erase(unsearched.begin());

	int x = currentNode.x;
	int y = currentNode.y;

	updatePos = glm::vec2(x, y);

	Node north = _maze[x][y - 1];
	if (north.type == Node::Type::UNSEARCHED && north.steps == -1)
	{
		north.parent = Node::Direction::SOUTH;
		north.steps = currentNode.steps + 1;
		north.type = Node::Type::SEARCHED;

		unsearched.push_back(north);
		_maze[x][y - 1] = north;
	}
	else if (north.type == Node::Type::TARGET)
	{
		north.parent = Node::Direction::SOUTH;
		north.steps = currentNode.steps + 1;
		_maze[x][y - 1] = north;

		//Clear unsearched so that it starts empty if the function is needed again
		unsearched.clear();
		return false;
	}

	Node south = _maze[x][y + 1];
	if (south.type == Node::Type::UNSEARCHED && south.steps == -1)
	{
		south.parent = Node::Direction::NORTH;
		south.steps = currentNode.steps + 1;
		south.type = Node::Type::SEARCHED;

		unsearched.push_back(south);
		_maze[x][y + 1] = south;
	}
	else if (south.type == Node::Type::TARGET)
	{
		south.parent = Node::Direction::NORTH;
		south.steps = currentNode.steps + 1;
		_maze[x][y + 1] = south;

		//Clear unsearched so that it starts empty if the function is needed again
		unsearched.clear();
		return false;
	}

	Node east = _maze[x + 1][y];
	if (east.type == Node::Type::UNSEARCHED && east.steps == -1)
	{
		east.parent = Node::Direction::WEST;
		east.steps = currentNode.steps + 1;
		east.type = Node::Type::SEARCHED;

		unsearched.push_back(east);
		_maze[x + 1][y] = east;
	}
	else if (east.type == Node::Type::TARGET)
	{
		east.parent = Node::Direction::WEST;
		east.steps = currentNode.steps + 1;
		_maze[x + 1][y] = east;

		//Clear unsearched so that it starts empty if the function is needed again
		unsearched.clear();
		return false;
	}

	Node west = _maze[x - 1][y];
	if (west.type == Node::Type::UNSEARCHED && west.steps == -1)
	{
		west.parent = Node::Direction::EAST;
		west.steps = currentNode.steps + 1;
		west.type = Node::Type::SEARCHED;

		unsearched.push_back(west);
		_maze[x - 1][y] = west;
	}
	else if (west.type == Node::Type::TARGET)
	{
		west.parent = Node::Direction::EAST;
		west.steps = currentNode.steps + 1;
		_maze[x - 1][y] = west;

		//Clear unsearched so that it starts empty if the function is needed again
		unsearched.clear();
		return false;
	}

	return true;
}

bool Maze::stepTrace(glm::vec2 &updatePos)
{
	static Node currentNode;
	if (currentNode.x == -1)
	{
		currentNode = getTarget();
	}

	if (currentNode.type == Node::Type::START)
	{
		currentNode = Node();
		return false;
	}

	updatePos = glm::vec2(currentNode.x, currentNode.y);

	//Mark the current node as traced as long as it is empty
	_maze[currentNode.x][currentNode.y].type = currentNode.type == Node::Type::SEARCHED ? Node::Type::TRACED : currentNode.type;

	switch (currentNode.parent)
	{
	case Node::Direction::NORTH:
		currentNode = _maze[currentNode.x][currentNode.y - 1];
		break;

	case Node::Direction::SOUTH:
		currentNode = _maze[currentNode.x][currentNode.y + 1];
		break;

	case Node::Direction::EAST:
		currentNode = _maze[currentNode.x + 1][currentNode.y];
		break;

	case Node::Direction::WEST:
		currentNode = _maze[currentNode.x - 1][currentNode.y];
		break;
	}

	return true;
}

int Maze::process()
{
	//Loop runs until it runs out of nodes to search or the solution is found
	while (stepProcess());

	return getTarget().steps;
}

void Maze::trace()
{
	while (stepTrace());
}

void Maze::reset()
{
	for (int i = 1; i < _maze.size() - 1; i++)
	{
		for (int o = 1; o < _maze[0].size() - 1; o++)
		{
			_maze[i][o].steps = -1;
			_maze[i][o].parent = Node::Direction::NONE;

			if (_maze[i][o].type == Node::Type::SEARCHED || _maze[i][o].type == Node::Type::TRACED)
			{
				_maze[i][o].type = Node::Type::UNSEARCHED;
			}
		}
	}
}

void Maze::getType(std::vector<std::vector<Node::Type>> &output) const
{
	output.clear();

	for (int i = 0; i < _maze.size(); i++)
	{
		std::vector<Node::Type> tmpVector;

		for (int o = 0; o < _maze[0].size(); o++)
		{
			tmpVector.push_back(_maze[i][o].type);
		}

		output.push_back(tmpVector);
	}
}

void Maze::print() const
{
	std::cout << std::endl;

	for (int o = 0; o < _maze[0].size(); o++)
	{
		for (int i = 0; i < _maze.size(); i++)
		{
			std::cout << (char)_maze[i][o].type;
		}

		std::cout << std::endl;
	}
}

void Maze::printParent() const
{
	std::cout << std::endl;

	for (int o = 0; o < _maze[0].size(); o++)
	{
		for (int i = 0; i < _maze.size(); i++)
		{
			if(_maze[i][o].type == Node::Type::SEARCHED) std::cout << _maze[i][o].parent;
			else std::cout << (char)_maze[i][o].type;
		}

		std::cout << std::endl;
	}
}

void Maze::generate(int xSize, int ySize)
{
	srand(time(nullptr));

	if (xSize < 1 || ySize < 1)
	{
		std::cout << "Given sizes too small, can't generate maze." << std::endl;
		return;
	}

	//Even numbers doubles the number of walls on the edges
	if (xSize % 2 == 0) xSize++;
	if (ySize % 2 == 0) ySize++;

	_maze.clear();

	//Fill the grid with empty nodes
	for (int i = 0; i < xSize + 2; i++)
	{
		std::vector<Node> tmpVec;
		for (int o = 0; o < ySize + 2; o++)
		{
			tmpVec.push_back(Node(Node::Type::WALL, i, o));
		}

		_maze.push_back(tmpVec);
	}

	//Choose start and target points that aren't too close together
	int startX = 0;
	int startY = 0;
	int targetX = 0;
	int targetY = 0;
	do
	{
		//Get random start and target
		startX = rand() % (xSize / 2);
		startY = rand() % (ySize / 2);
		startX *= 2;
		startY *= 2;
		startX += 1;
		startY += 1;

		targetX = rand() % (xSize / 2);
		targetY = rand() % (ySize / 2);
		targetX *= 2;
		targetY *= 2;
		targetX += 1;
		targetY += 1;
	} while (targetX == startX && targetY == startY && distance(startX, startY, targetX, targetY) > distance(0, 0, xSize, ySize) / 2);

	_maze[startX][startY].type = Node::Type::START;
	_maze[targetX][targetY].type = Node::Type::TARGET;

	//Start generation at the maze start
	std::vector<Node> stack;
	stack.push_back(getStart());

	//Generate the maze
	while (stack.size() != 0)
	{
		Node back = stack.back();

		Node::Direction dir = Node::Direction::NONE;
		std::vector<Node::Direction> validDirs;

		if (back.y > 1 && _maze[back.x][back.y - 2].type == Node::Type::WALL) validDirs.push_back(Node::Direction::NORTH);
		if (back.y < ySize - 1 && _maze[back.x][back.y + 2].type == Node::Type::WALL) validDirs.push_back(Node::Direction::SOUTH);
		if (back.x < xSize - 1 && _maze[back.x + 2][back.y].type == Node::Type::WALL) validDirs.push_back(Node::Direction::EAST);
		if (back.x > 1 && _maze[back.x - 2][back.y].type == Node::Type::WALL) validDirs.push_back(Node::Direction::WEST);

		//If no valid direction then continue without setting one
		if (validDirs.size() != 0) dir = validDirs[rand() % validDirs.size()];

		switch (dir)
		{
			case Node::Direction::NORTH:
				_maze[back.x][back.y - 1].type = Node::Type::UNSEARCHED;
				_maze[back.x][back.y - 2].type = Node::Type::UNSEARCHED;

				//Branch off of the next point before continuing on this point
				stack.push_back(_maze[back.x][back.y - 2]);
				continue;

			case Node::Direction::SOUTH:
				_maze[back.x][back.y + 1].type = Node::Type::UNSEARCHED;
				_maze[back.x][back.y + 2].type = Node::Type::UNSEARCHED;

				stack.push_back(_maze[back.x][back.y + 2]);
				continue;

			case Node::Direction::EAST:
				_maze[back.x + 1][back.y].type = Node::Type::UNSEARCHED;
				_maze[back.x + 2][back.y].type = Node::Type::UNSEARCHED;

				stack.push_back(_maze[back.x + 2][back.y]);
				continue;

			case Node::Direction::WEST:
				_maze[back.x - 1][back.y].type = Node::Type::UNSEARCHED;
				_maze[back.x - 2][back.y].type = Node::Type::UNSEARCHED;

				stack.push_back(_maze[back.x - 2][back.y]);
				continue;

			default:
				stack.pop_back();
		}
	}

	//Connect the target to the maze
	Node target = getTarget();

	Node::Direction dir = Node::Direction::NONE;
	std::vector<Node::Direction> validDirs;

	if (target.y > 1 && _maze[target.x][target.y - 2].type == Node::Type::UNSEARCHED) validDirs.push_back(Node::Direction::NORTH);
	if (target.y < ySize - 1 && _maze[target.x][target.y + 2].type == Node::Type::UNSEARCHED) validDirs.push_back(Node::Direction::SOUTH);
	if (target.x < xSize - 1 && _maze[target.x + 2][target.y].type == Node::Type::UNSEARCHED) validDirs.push_back(Node::Direction::EAST);
	if (target.x > 1 && _maze[target.x - 2][target.y].type == Node::Type::UNSEARCHED) validDirs.push_back(Node::Direction::WEST);

	dir = validDirs[rand() % validDirs.size()];

	switch (dir)
	{
		case Node::Direction::NORTH:
			_maze[target.x][target.y - 1].type = Node::Type::UNSEARCHED;
			break;

		case Node::Direction::SOUTH:
			_maze[target.x][target.y + 1].type = Node::Type::UNSEARCHED;
			break;

		case Node::Direction::EAST:
			_maze[target.x + 1][target.y].type = Node::Type::UNSEARCHED;
			break;

		case Node::Direction::WEST:
			_maze[target.x - 1][target.y].type = Node::Type::UNSEARCHED;
			break;
	}
}

int Maze::distance(int x1, int y1, int x2, int y2) const
{
	double distance = sqrt((double)pow(x1 - x2, 2) + (double)pow(y1 - y2, 2));

	return (int)distance;
}
