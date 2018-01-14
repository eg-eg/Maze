#ifndef MAZE_H
#define MAZE_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <glm/vec2.hpp>

struct Node
{
	enum Direction
	{
		NONE = 0,
		NORTH = 1,
		SOUTH = 2,
		EAST = 4,
		WEST = 8
	};
	enum Type
	{
		SEARCHED = '^',
		START = '0',
		TARGET = '!',
		TRACED = '*',
		UNSEARCHED = '.',
		WALL = '#'
	};

	int x;
	int y;
	int steps = -1;	//Steps from the start

	//Distance from end + steps
	int cost = -1;

	Direction parent = NONE;
	Type type;

	Node()
	{
		//Set values to something that breaks if not changed later
		type = UNSEARCHED;
		x = -1;
		y = -1;
	}

	Node(Type typeIn, int xIn, int yIn)
	{
		type = valid(typeIn) ? typeIn : Type::UNSEARCHED;
		x = xIn;
		y = yIn;
	}

private:
	bool valid(Type typeIn)
	{
		if (typeIn == Type::UNSEARCHED || typeIn == Type::SEARCHED || typeIn == Type::START || typeIn == Type::TARGET || typeIn == Type::TRACED || typeIn == Type::WALL) return true;
		return false;
	}
};

class Maze
{
public:
	Maze();
	Maze(const std::string path);

	int getX() const { return _maze.size(); }
	int getY() const { return _maze[0].size(); }

	int getSteps() const { return getTarget().steps; }

	Node getStart() const;
	Node getTarget() const;

	void load(const std::string &path);

	bool stepProcess();
	bool stepTrace();

	bool stepProcess(glm::vec2 &updatePos);
	bool stepTrace(glm::vec2 &updatePos);

	int process();
	void trace();

	void reset();

	Node::Type getType(int x, int y) const { return _maze[x][y].type; };
	void getType(std::vector<std::vector<Node::Type>> &output) const;

	void print() const;
	void printParent() const;

	void generate(int xSize, int ySize);

private:
	int distance(int x1, int y1, int x2, int y2) const;

	void heapAdd(std::vector<Node> &heap, Node toAdd);
	Node heapPop(std::vector<Node> &heap);

	std::vector<std::vector<Node>> _maze;

	bool solved = false;
};

#endif