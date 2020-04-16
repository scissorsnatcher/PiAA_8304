#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <cassert>

bool debug = false;

struct Vertex;
struct Edge {
    Vertex *source = nullptr, *target = nullptr;
    int capacity = 0;
    int flow = 0;
    bool isReverseEdge = false;
    Edge *reverseEdge = nullptr;
};
struct EdgePtrComparator {
    bool operator()(const Edge *a, const Edge *b) const;
};
struct Vertex {
    char name = '?';
    bool inPath = false;
    std::set<Edge*, EdgePtrComparator> edges;
};
bool EdgePtrComparator::operator()(const Edge *a, const Edge *b) const {
    if (a->source->name != b->source->name)
        return a->source->name < b->source->name;
    if (a->target->name != b->target->name)
        return a->target->name < b->target->name;
    return a < b;
}
using Path = std::vector<Edge *>;
struct VertexPtrComparator {
    bool operator()(const Vertex *a, const Vertex *b) const {
        return a->name < b->name;
    }
};
struct Graph {
    Vertex *source = nullptr, *target = nullptr;
    std::set<Vertex *, VertexPtrComparator> vertices;
};

bool findPath(Vertex *source, Vertex *target, Path &path, bool clearPath = true) {
    if (clearPath) {
        path.clear();
        source->inPath = true;
    }
    for (auto edgeItr = source->edges.rbegin(); edgeItr != source->edges.rend(); edgeItr++) {
        auto edge = *edgeItr;
        if (!edge->target->inPath && edge->capacity > 0) {
            path.push_back(edge);
            if (edge->target == target)
                return true;
            edge->target->inPath = true;
            bool pathIsFound = findPath(edge->target, target, path, false);
            edge->target->inPath = false;
            if (pathIsFound) {
                source->inPath = false;
                return true;
            }
            path.pop_back();
        }
    }
    source->inPath = false;
    return false;
}

int findMinCapacity(const Path &path) {
    assert(!path.empty());
    int minCapacity = path.front()->capacity;
    for (auto &edge: path)
        minCapacity = std::min(minCapacity, edge->capacity);
    return minCapacity;
}

void changeFlow(const Path &path, int flowChange) {
    assert(flowChange >= 0);
    for (auto &edge: path) {
        edge->flow += flowChange;
        assert(edge->capacity >= flowChange);
        edge->capacity -= flowChange;

        edge->reverseEdge->flow -= flowChange;
        edge->reverseEdge->capacity += flowChange;
    }
}

void addReverseEdges(Graph &graph) {
    for (auto &vertex : graph.vertices) {
        for (auto &edge : vertex->edges) {
            assert(!(edge->isReverseEdge && edge->reverseEdge == nullptr));
            if (edge->reverseEdge != nullptr)
                continue;

            auto reverseEdge = new Edge;
            reverseEdge->reverseEdge = edge;
            reverseEdge->isReverseEdge = true;
            reverseEdge->target = edge->source;
            reverseEdge->source = edge->target;

            edge->reverseEdge = reverseEdge;
            edge->target->edges.insert(reverseEdge);
        }
    }
}

void printGraph(const Graph &graph, bool printReverseEdges = true, bool printProperties = true) {
    for (auto &vertex : graph.vertices) {
        for (auto &edge : vertex->edges) {
            if (edge->isReverseEdge && !printReverseEdges)
                continue;
            std::cout << vertex->name << " " << edge->target->name;
            if (printProperties) {
                std::cout
                        << " {capacity: " << edge->capacity
                        << ", flow: " << edge->flow
                        << ", isReverseEdge: " << (edge->isReverseEdge ? "true" : "false")
                        << "}";
            }
            else {
                std::cout << " " << edge->flow;
            }
            std::cout << std::endl;
        }
    }
}

void printPath(const Path &path) {
    for (auto &edge : path) {
        std::cout << edge->source->name << " ";
    }
    assert(!path.empty());
    std::cout << path.back()->target->name << std::endl;
}

int findMaxFlow(Graph &graph) {
    if (debug) std::cout << "Adding reverse edges:" << std::endl;
    addReverseEdges(graph);
    if (debug) printGraph(graph);

    int maxFlow = 0;
    Path path;

    if (debug) std::cout << "Searching a path." << std::endl;
    while (findPath(graph.source, graph.target, path)) {
        if (debug) {
            std::cout << "Path is found: " << std::endl;
            printPath(path);
        }

        int minCapacity = findMinCapacity(path);
        if (debug) std::cout << "Min capacity = " << minCapacity << std::endl;

        if (debug) std::cout << "Changing the flow through the path." << std::endl;
        changeFlow(path, minCapacity);
        if (debug) {
            std::cout << "Modified graph:" << std::endl;
            printGraph(graph);
        }

        maxFlow += minCapacity;
        if (debug) std::cout << "Flow value = " << maxFlow << std::endl;
    }
    if (debug) std::cout << "Path is not found - the algorithm is complete." << std::endl;
    return maxFlow;
}

Graph readGraph() {
    int vertexNumber;
    std::cin >> vertexNumber;

    char sourceName, targetName;
    std::cin >> sourceName >> targetName;

    std::map<char, Vertex *> vertices;
    for (int i = 0; i < vertexNumber; i++) {
        char edgeSourceName, edgeTargetName;
        int capacity;
        std::cin >> edgeSourceName >> edgeTargetName >> capacity;

        auto &edgeSource = vertices[edgeSourceName];
        if (edgeSource == nullptr)
            edgeSource = new Vertex{edgeSourceName};
        auto &edgeTarget = vertices[edgeTargetName];
        if (edgeTarget == nullptr)
            edgeTarget = new Vertex{edgeTargetName};

        auto edge = new Edge;
        edge->source = edgeSource;
        edge->target = edgeTarget;
        edge->capacity = capacity;
        edgeSource->edges.insert(edge);
    }

    Graph graph;
    graph.source = vertices.at(sourceName);
    graph.target = vertices.at(targetName);
    for (auto &vertexWithName : vertices) {
        Vertex *vertex = vertexWithName.second;
        graph.vertices.insert(vertex);
    }

    return graph;
}

int main(int ac, char** av) {
    if (ac > 1)
        debug = true;

    auto graph = readGraph();

    int maxFlow = findMaxFlow(graph);

    std::cout << maxFlow << std::endl;
    printGraph(graph, false, false);

    return 0;
}
