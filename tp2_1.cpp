#include <iostream>
#include <algorithm>
#include <cl.hpp>
#include <fstream>
#include <utility>
#include <sstream>
#include <chrono>
#include <queue>
// OpenCL kernel code to be executed in parallel
const std::string KERNEL_CODE =
	"void kernel maze_solver(global int* maze, global int* parents, global int* completion, int rows, int columns) {"
		"int id = get_global_id(0);"
		"int unreachable = 0;"
		"while ((maze[id] == 0 || maze[id] == 2) && completion[0] == 0 && unreachable == 0) {"
			"if (maze[id] == 2) {"
				"int dx[4] = {0, 0, 1, -1};"
				"int dy[4] = {1, -1, 0, 0};"
				"for (int d = 0; d < 4; ++d) {"
					"int nx = id / columns + dx[d];"
					"int ny = id % columns + dy[d];"
					"if (nx >= 0 && nx < rows && ny >= 0 && ny < columns) {"
						"int neighbor_index = nx * columns + ny;"
						"if (maze[neighbor_index] == 0) {"
							"maze[neighbor_index] = 2;"
							"parents[neighbor_index] = id;"
						"} else if (maze[neighbor_index] == 5) {"
							"completion[0] = 1;"
							"completion[1] = neighbor_index;"
							"parents[neighbor_index] = id;"
						"}"
					"}"
				"}"
				"maze[id] = 3;"
			"}"
			"barrier(CLK_GLOBAL_MEM_FENCE);"
			"if (completion[0] == 0) {"
				"int all_explored = 1;"
				"for (int i = 0; i < rows * columns; ++i) {"
					"if (maze[i] == 2) {"
						"all_explored = 0;"
						"break;"
					"}"
				"}"
				"if (all_explored) {"
					"unreachable = 1;"
				"}"
			"}"
		"}"
	"};";
// Function to print the maze grid with specific symbols for cells
void printMaze(const std::vector<int>& maze, int rows, int columns) {
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			int index = i * columns + j;
			if (maze[index] == 4) {
				std::cout << "S "; // Start cell
			}
			else if (maze[index] == 5) {
				std::cout << "D "; // Destination cell
			}
			else if (maze[index] == 3) {
				std::cout << "X "; // Explored cell
			}
			else {
				std::cout << (maze[index] ? "# " : ". "); // Wall or empty cell
			}
		}
		std::cout << std::endl;
	}
}
// Function to print the maze grid with numeric values
void printMazeNumeric(const std::vector<int>& maze, int rows, int columns) {
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			int index = i * columns + j;
			std::cout << maze[index] << "\t";
		}
		std::cout << std::endl;
	}
}
// Function to read maze data from the input file
std::pair<int, std::vector<int>> read_maze(std::ifstream& inputFile) {
	int mazeSize;
	inputFile >> mazeSize;
	std::vector<int> mazeContent;
	std::string line;
	while (std::getline(inputFile, line)) {
		for (char character : line) {
			if (character == '0') {
				mazeContent.push_back(0); // Empty cell
			} else if (character == '1') {
				mazeContent.push_back(1); // Wall cell
			} else if (character == 'S') {
				mazeContent.push_back(4); // Start cell
			} else if (character == 'D') {
				mazeContent.push_back(5); // Destination cell
			}
		}
	}
	return std::make_pair(mazeSize, mazeContent);
}
// Function to find the start and destination cell in the maze
std::pair<int, int> find_endpoints(std::vector<int> maze) {
	int start_cell = -1;
	int destination_cell = -1;
	for (size_t i = 0; i < maze.size(); ++i) {
		if (maze[i] == 4) start_cell = i;
		if (maze[i] == 5) destination_cell = i;
		if (start_cell != -1 && destination_cell != -1) break;
	}
	return std::make_pair(start_cell, destination_cell);
}
// Function to solve the maze using a sequential Breadth-First Search (BFS)
std::pair<bool, std::vector<int>> maze_solver_sequential(std::vector<int> maze, int rows, int columns, int start_cell, int destination_cell) {
	std::vector<int> parents(rows * columns, -1);
	std::queue<int> bfs_queue;
	bfs_queue.push(start_cell);
	while (!bfs_queue.empty()) {
		int current_cell = bfs_queue.front();
		bfs_queue.pop();
		if (current_cell == destination_cell) {
			std::vector<int> path;
			while (current_cell != start_cell) {
				path.push_back(current_cell);
				current_cell = parents[current_cell];
			}
			path.push_back(start_cell);
			std::reverse(path.begin(), path.end());
			return std::make_pair(true, path);
		}
		maze[current_cell] = 3;
		int dx[4] = {0, 0, 1, -1};
		int dy[4] = {1, -1, 0, 0};
		for (int d = 0; d < 4; ++d) {
			int nx = current_cell / columns + dx[d];
			int ny = current_cell % columns + dy[d];
			int neighbor_index = nx * columns + ny;
			if (nx >= 0 && nx < rows && ny >= 0 && ny < columns && maze[neighbor_index] == 0) {
				bfs_queue.push(neighbor_index);
				parents[neighbor_index] = current_cell;
			}
		}
	}
	return std::make_pair(false, std::vector<int>());
}
int main() {
	// Open the input file
	std::ifstream inputFile(".../10.txt");
	if (!inputFile.is_open()) {
		std::cout << "Error opening the file." << std::endl;
		exit(1);
	}
	// Read maze data from the input file
	std::pair<int, std::vector<int>> mazeData = read_maze(inputFile);
	int rows = mazeData.first;
	int columns = mazeData.first;
	std::vector<int> maze = mazeData.second;
	// Find the start and destination cell in the maze
	std::pair<int, int> endpoints = find_endpoints(maze);
	// Print the maze before solving
	std::cout << "Maze:" << std::endl;
	printMaze(maze, rows, columns);
	// Set the start cell as explored
	maze[endpoints.first] = 2;
	// Initialize arrays for parallel processing
	std::vector<int> parents(rows * columns, 0);
	std::vector<int> completion(2, 0);
	// Initialize OpenCL environment
	std::vector<cl::Platform> available_platforms;
	cl::Platform::get(&available_platforms);
	if (available_platforms.size() == 0) {
		std::cout << "No platform found." << std::endl;
		exit(1);
	}
	cl::Platform default_platform = available_platforms[0];
	std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
	std::vector<cl::Device> available_devices;
	default_platform.getDevices(CL_DEVICE_TYPE_ALL, &available_devices);
	cl::Device default_device = available_devices[0];
	std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << std::endl;
	cl::Context context({default_device});
	cl::Program::Sources sources;
	sources.push_back({KERNEL_CODE.c_str(), KERNEL_CODE.length()});
	cl::Program program(context, sources);
	if (program.build({default_device}) != CL_SUCCESS) {
		std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
		exit(1);
	}
	// Set up OpenCL buffers and kernels arguments
	cl::Kernel maze_solver_kernel(program, "maze_solver");
	cl::Buffer maze_buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int)*maze.size(), maze.data());
	cl::Buffer parents_buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * parents.size(), parents.data());
	cl::Buffer completion_buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * completion.size(), completion.data());
	maze_solver_kernel.setArg(0, maze_buffer);
	maze_solver_kernel.setArg(1, parents_buffer);
	maze_solver_kernel.setArg(2, completion_buffer);
	maze_solver_kernel.setArg(3, rows);
	maze_solver_kernel.setArg(4, columns);
	// Craete the OpenCL command queue and execute the kernel
	cl::CommandQueue queue(context, default_device);
	auto start_time = std::chrono::high_resolution_clock::now();
	cl_int status = queue.enqueueNDRangeKernel(maze_solver_kernel, cl::NullRange, cl::NDRange(rows * columns));
	if (status != CL_SUCCESS) {
		std::cout << "NDRange Exception: " << status << std::endl;
		exit(1);
	}
	queue.finish();
	// Retrieve results from OpenCL buffers
	std::vector<int> completion_results(completion.size());
	status = queue.enqueueReadBuffer(completion_buffer, CL_TRUE, 0, sizeof(int) * completion_results.size(), completion_results.data());
	if (status != CL_SUCCESS) {
		std::cout << "Read Exception 1: " << status << std::endl;
		exit(1);
	}
	// Extract the path from the parents array
	std::cout << "Destination: " << completion_results[1] << std::endl;
	std::vector<int> parents_results(parents.size());
	status = queue.enqueueReadBuffer(parents_buffer, CL_TRUE, 0, sizeof(int) * parents_results.size(), parents_results.data());
	if (status != CL_SUCCESS) {
		std::cout << "Read Exception 2: " << status << std::endl;
		exit(1);
	}
	std::vector<int> path;
	int current_cell = completion_results[1];
	while (current_cell != endpoints.first) {
		path.push_back(current_cell);
		current_cell = parents_results[current_cell];
	}
	// Print out the parth found by the parallel algorithm
	path.push_back(endpoints.first);
	std::reverse(path.begin(), path.end());
	std::cout << "Path: ";
	for (int i = 0; i < path.size(); ++i) {
		int row = path[i] / columns;
		int col = path[i] % columns;
		std::cout << "(" << row << "," << col << ") ";
	}
	auto parallel_end_time = std::chrono::high_resolution_clock::now();
	auto parallel_duration = std::chrono::duration_cast<std::chrono::milliseconds>(parallel_end_time - start_time);
	std::cout << "Time taken by parallel algorithm: " << parallel_duration.count() << "ms" << std::endl;
	// Solve the maze using a sequential BFS algorithm
	auto start_time_sequential_bfs = std::chrono::high_resolution_clock::now();
	std::pair<bool, std::vector<int>> result_sequential_bfs = maze_solver_sequential(maze, rows, columns, endpoints.first, endpoints.second);
	if (result_sequential_bfs.first) {
		std::vector<int> path_sequential_bfs = result_sequential_bfs.second;
		std::cout << "Path: ";
		for (int i = 0; i < path_sequential_bfs.size(); ++i) {
			int row = path_sequential_bfs[i] / columns;
			int col = path_sequential_bfs[i] % columns;
			std::cout << "(" << row << "," << col << ") ";
		}
		std::cout << std::endl;
	}
	auto end_time_sequential_bfs = std::chrono::high_resolution_clock::now();
	auto sequential_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_sequential_bfs - start_time_sequential_bfs);
	std::cout << "Time taken by sequential algorithm: " << sequential_duration.count() << "ms" << std::endl;
	exit(0);
}