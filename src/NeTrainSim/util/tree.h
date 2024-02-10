#ifndef TREE_H
#define TREE_H

#include <vector>
#include <iostream>
#include <queue>
#include <limits>

template <typename T>
class TreeNode {
public:
    T value;
    double cost;
    TreeNode<T>* parent; // Parent pointer
    std::vector<TreeNode<T>*> children = std::vector<TreeNode<T>*>();


    TreeNode(T val, double c = std::numeric_limits<double>::max(),
             TreeNode<T> *parent = nullptr) : value(val),
        cost(c), parent(parent) {}

    void addChild(TreeNode<T>* child) {
        child->parent = this;
        children.push_back(child);
    }

    ~TreeNode() {
        if (children.size() < 1) return;
        for (auto child : children) {
            if (child) {
                delete child; // Just delete the object pointed to by the pointer
            }
            child = nullptr;
        }
    }

    // Copy constructor
    TreeNode(const TreeNode& other) : value(other.value), cost(other.cost), parent(nullptr) {
        for (auto child : other.children) {
            addChild(new TreeNode<T>(*child)); // Deep copy of children
        }
    }

    // Copy assignment operator
    TreeNode& operator=(const TreeNode& other) {
        if (this != &other) { // Protect against self-assignment
            value = other.value;
            cost = other.cost;

            // Clear existing children
            for (auto child : children) {
                delete child;
            }
            children.clear();

            // Copy new children
            for (auto child : other.children) {
                addChild(new TreeNode<T>(*child)); // Deep copy
            }
        }
        return *this;
    }
};

template <typename T>
class Tree {
private:
    TreeNode<T>* root;
    int depth;



    void depthFirstTraversal(TreeNode<T>* node) {
        if (node != nullptr) {
            std::cout << node->value << " ";
            for (auto child : node->children) {
                depthFirstTraversal(child);
            }
        }
    }

    void clearCostsHelper(TreeNode<T>* node) {
        if (node != nullptr) {
            node->cost = std::numeric_limits<double>::max(); // Reset the cost
            for (auto child : node->children) {
                clearCostsHelper(child); // Recursively clear costs for all children
            }
        }
    }

    bool insertChildHelper(TreeNode<T>* currentNode, TreeNode<T>* parentNode,
                           TreeNode<T>* childNode)
    {
        if (currentNode == parentNode) {
            currentNode->addChild(childNode);
            return true;
        }

        for (auto& child : currentNode->children) {
            if (insertChildHelper(child, parentNode, childNode)) {
                return true;
            }
        }

        return false; // Parent node not found in the tree
    }

    // Private method to update the depth of the tree
    void updateDepth() {
        depth = calculateDepth(root);
    }

    // Private method to update the depth of the tree
    int calculateDepth(TreeNode<T>* node) {
        if (!node) return -1;
        int maxChildDepth = -1;
        for (auto child : node->children) {
            maxChildDepth = std::max(maxChildDepth, calculateDepth(child));
        }
        return 1 + maxChildDepth;
    }

    // Helper function to find all leaf nodes and their costs
    void findLeafNodesAndCosts(TreeNode<T>* node,
                               std::vector<std::pair<TreeNode<T> *, double> > &leafNodes)
    {
        if (!node) return;
        // If it's a leaf node, add it and its cost to the list
        if (node->children.empty()) {
            leafNodes.emplace_back(node, node->cost);
            return;
        }
        // Otherwise, continue searching in children
        for (auto child : node->children) {
            findLeafNodesAndCosts(child, leafNodes);
        }
    }

public:
    Tree() : root(nullptr), depth(0) { }

    ~Tree() {
        clear();
    }

    // Copy constructor
    Tree(const Tree& other) : root(nullptr), depth(other.depth) {
        if (other.root) {
            root = new TreeNode<T>(*other.root); // Deep copy of the root node
            updateDepth(); // Ensure the depth is updated correctly
        }
    }

    // Copy assignment operator
    Tree& operator=(const Tree& other) {
        if (this != &other) { // Protect against self-assignment
            clear(); // Clear existing tree
            if (other.root) {
                root = new TreeNode<T>(*other.root); // Deep copy of the root
                updateDepth(); // Update depth
            }
            depth = other.depth;
        }
        return *this;
    }

    void setRoot(T data) {
        clear();
        root = new TreeNode<T>(data, 0.0, nullptr);
        updateDepth();
    }

    void setRoot(TreeNode<T>* rootNode) {
        root = rootNode;
        updateDepth();
    }

    TreeNode<T>* getRoot() {
        return root; // Update depth whenever the root is set
    }

    void depthFirst() {
        depthFirstTraversal(root);
    }

    /**
     * Finds and returns the child of a given parent node with the minimum cost.
     *
     * @param parent A pointer to the parent TreeNode whose children
     *          are to be examined. Must not be nullptr.
     * @return A pointer to the TreeNode with the minimum cost among the
     *          children of the specified parent.
     *         Returns nullptr if the parent is nullptr, has no children,
     *         or in case of an empty tree.
     *
     * Note: The caller must ensure that the returned pointer is used in
     *          a manner that respects the tree's current structure and
     *          state. The tree should not be modified while using the
     *          returned pointer, and the caller should be aware of the
     *          possibility of the tree changing after the pointer is obtained.
     */
    TreeNode<T>* getMinCostNodeOfParent(TreeNode<T>* parent)
    {
        // Check for null or empty parent
        if (parent == nullptr || parent->children.empty()) {
            return nullptr;
        }

        if (parent->children.size() == 0)
        {
            return nullptr;
        }

        double minCost = std::numeric_limits<double>::max();
        TreeNode<T>* node = nullptr;
        for (auto& child: parent->children)
        {
            if (child->cost < minCost)
            {
                minCost = child->cost;
                node = child;
            }
        }
        return node;
    }

    // Utility function to find a node in the tree
    bool findNode(TreeNode<T>* currentNode, TreeNode<T>* targetNode) {
        if (!currentNode) {
            return false; // Base case: reached a leaf or empty subtree
        }
        if (currentNode == targetNode) {
            return true; // Node found
        }
        // Recursively search in children
        for (auto child : currentNode->children) {
            if (findNode(child, targetNode)) {
                return true; // Node found in the subtree
            }
        }
        return false; // Node not found in this subtree
    }

    void clear() {
        if (root)
        {
            delete root; // Delete the root node, which triggers recursive deletion of all child nodes
            root = nullptr; // to avoid dangling pointers
        }
    }

    void clearCosts() {
        clearCostsHelper(root);
    }

    bool insertChild(TreeNode<T>* parentNode, TreeNode<T>* childNode) {
        if (parentNode == nullptr || childNode == nullptr) {
            return false; // Invalid input
        }
        if (root == nullptr) {
            return false; // Tree is empty
        }
        bool result = insertChildHelper(root, parentNode, childNode);
        // reduce the number of updating to only the first child
        if (parentNode->children.size() < 2)
        {
            updateDepth();
        }
        return result;
    }

    bool insertChild(TreeNode<T>* parentNode, T childData) {
        if (!parentNode) {
            // parentNode is nullptr, insertion is not possible
            return false;
        }

        // Check if the tree is empty or parentNode does not belong to this tree
        if (!root || !findNode(root, parentNode)) {
            // The tree is empty or parentNode is not part of this tree
            return false;
        }

        // Create a new TreeNode for childData
        TreeNode<T>* newChild = new TreeNode<T>(childData);

        // Insert the new child under the parentNode
        parentNode->addChild(newChild);

        return true; // Indicate successful insertion
    }

    std::vector<TreeNode<T>*> getNodesAtLevel(int level)
    {
        std::vector<TreeNode<T>*> nodesAtLevel;
        if (!root || level < 1) return nodesAtLevel; // No nodes at level < 1

        std::queue<TreeNode<T>*> queue;
        queue.push(root);
        int currentLevel = 0;

        while (!queue.empty()) {
            int levelSize = queue.size(); // Number of nodes at the current level

            for (int i = 0; i < levelSize; ++i) {
                TreeNode<T>* currentNode = queue.front();
                queue.pop();

                // Adjusted for level starting from 1 for children of root
                if (currentLevel == level - 1) {
                    nodesAtLevel.push_back(currentNode);
                }

                // Add children to the queue for the next level
                for (auto child : currentNode->children) {
                    queue.push(child);
                }
            }

            if (currentLevel == level - 1) break; // Stop after reaching the target level

            currentLevel++;
        }

        return nodesAtLevel;
    }

    int getDepth() const {
        return depth; // Provide a method to access the depth
    }

    // Function to get the shortest path and its cost
    std::pair<std::vector<TreeNode<T>*>, double> getShortestPathAndCost()
    {
        std::vector<TreeNode<T>*> shortestPath;
        double shortestPathCost = std::numeric_limits<double>::max();
        std::vector<std::pair<TreeNode<T>*, double>> leafNodes;

        // Find all leaf nodes and their costs
        findLeafNodesAndCosts(root, leafNodes);

        // Determine the leaf node with the minimum cost
        TreeNode<T>* minCostLeaf = nullptr;
        for (const auto& leafNode : leafNodes) {
            if (leafNode.second < shortestPathCost) {
                shortestPathCost = leafNode.second;
                minCostLeaf = leafNode.first;
            }
        }

        // Trace back from the leaf node with minimum cost to the root
        while (minCostLeaf) {
            shortestPath.insert(shortestPath.begin(), minCostLeaf);
            minCostLeaf = minCostLeaf->parent;
        }

        return {shortestPath, shortestPathCost};
    }

};


#endif // TREE_H
