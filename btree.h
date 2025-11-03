#ifndef BTree_H
#define BTree_H

#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include <queue>
#include "node.h"

using namespace std;

template <typename TK>
class BTree {
 private:
  Node<TK>* root;
  int M;  // grado u orden del arbol
  int n; // total de elementos en el arbol 

  void splitChild(Node<TK>* parent, int idx) {
    Node<TK>* y = parent->children[idx];
    Node<TK>* z = new Node<TK>(M);
    z->leaf = y->leaf;
    int maxKeys = M - 1;
    int t = maxKeys / 2;

    int j = 0;
    for (int i = t + 1; i < maxKeys; ++i) {
      z->keys[j++] = y->keys[i];
    }
    z->count = j;

    if (!y->leaf) {
      int cj = 0;
      for (int i = t + 1; i <= maxKeys; ++i) {
        z->children[cj++] = y->children[i];
        y->children[i] = nullptr;
      }
    }

    y->count = t;

    for (int i = parent->count + 1; i > idx + 1; --i) {
      parent->children[i] = parent->children[i - 1];
    }
    parent->children[idx + 1] = z;

    for (int i = parent->count; i > idx; --i) {
      parent->keys[i] = parent->keys[i - 1];
    }
    parent->keys[idx] = y->keys[t];
    parent->count += 1;

    void insertNonFull(Node<TK>* node, TK key) {
    int i = node->count - 1;
    if (node->leaf) {
      // move keys to make room
      while (i >= 0 && node->keys[i] > key) {
        node->keys[i + 1] = node->keys[i];
        i--;
      }
      node->keys[i + 1] = key;
      node->count += 1;
      n += 1;
    } else {
      while (i >= 0 && node->keys[i] > key) i--;
      int childIdx = i + 1;
      if (node->children[childIdx]->count == M - 1) {
        splitChild(node, childIdx);
        if (node->keys[childIdx] < key) childIdx++;
      }
      insertNonFull(node->children[childIdx], key);
    }
  }

  bool searchNode(Node<TK>* node, TK key) {
    if (!node) return false;
    int i = 0;
    while (i < node->count && key > node->keys[i]) i++;
    if (i < node->count && node->keys[i] == key) return true;
    if (node->leaf) return false;
    return searchNode(node->children[i], key);
  }

  void inorder(Node<TK>* node, vector<TK>& out) {
    if (!node) return;
    for (int i = 0; i < node->count; ++i) {
      if (!node->leaf) inorder(node->children[i], out);
      out.push_back(node->keys[i]);
    }
    if (!node->leaf) inorder(node->children[node->count], out);
  }

  void rangeCollect(Node<TK>* node, TK begin, TK end, vector<TK>& out) {
    if (!node) return;
    int i = 0;
    while (i < node->count && node->keys[i] < begin) i++;
    for (int j = 0; j <= node->count; ++j) {
      if (!node->leaf) {
        if (j <= i) {
          rangeCollect(node->children[j], begin, end, out);
        } else {
          rangeCollect(node->children[j], begin, end, out);
        }
      }
      if (j < node->count) {
        if (node->keys[j] >= begin && node->keys[j] <= end) out.push_back(node->keys[j]);
      }
    }
  }

  int findKey(Node<TK>* node, TK key) {
    int idx = 0;
    while (idx < node->count && node->keys[idx] < key) ++idx;
    return idx;
  }

  TK getPredecessor(Node<TK>* node, int idx) {
    Node<TK>* cur = node->children[idx];
    while (!cur->leaf) cur = cur->children[cur->count];
    return cur->keys[cur->count - 1];
  }
  TK getSuccessor(Node<TK>* node, int idx) {
    Node<TK>* cur = node->children[idx + 1];
    while (!cur->leaf) cur = cur->children[0];
    return cur->keys[0];
  }

  void fill(Node<TK>* node, int idx) {
    int minKeys = (int)ceil((double)M / 2.0) - 1;
    if (idx != 0 && node->children[idx - 1]->count >= minKeys + 1) {
      borrowFromPrev(node, idx);
    } else if (idx != node->count && node->children[idx + 1]->count >= minKeys + 1) {
      borrowFromNext(node, idx);
    } else {
      if (idx != node->count) {
        merge(node, idx);
      } else {
        merge(node, idx - 1);
      }
    }
  }

  void borrowFromPrev(Node<TK>* node, int idx) {
    Node<TK>* child = node->children[idx];
    Node<TK>* sibling = node->children[idx - 1];
    for (int i = child->count - 1; i >= 0; --i)
      child->keys[i + 1] = child->keys[i];
    if (!child->leaf) {
      for (int i = child->count; i >= 0; --i)
        child->children[i + 1] = child->children[i];
    }
    child->keys[0] = node->keys[idx - 1];
    if (!child->leaf)
      child->children[0] = sibling->children[sibling->count];
    node->keys[idx - 1] = sibling->keys[sibling->count - 1];
    child->count += 1;
    sibling->count -= 1;
  }

  void borrowFromNext(Node<TK>* node, int idx) {
    Node<TK>* child = node->children[idx];
    Node<TK>* sibling = node->children[idx + 1];
    child->keys[child->count] = node->keys[idx];
    if (!child->leaf)
      child->children[child->count + 1] = sibling->children[0];
    node->keys[idx] = sibling->keys[0];
    for (int i = 1; i < sibling->count; ++i)
      sibling->keys[i - 1] = sibling->keys[i];
    if (!sibling->leaf) {
      for (int i = 1; i <= sibling->count; ++i)
        sibling->children[i - 1] = sibling->children[i];
    }
    child->count += 1;
    sibling->count -= 1;
  }

  void merge(Node<TK>* node, int idx) {
    Node<TK>* child = node->children[idx];
    Node<TK>* sibling = node->children[idx + 1];
    int minKeys = (int)ceil((double)M / 2.0) - 1;
    child->keys[child->count] = node->keys[idx];
    for (int i = 0; i < sibling->count; ++i)
      child->keys[child->count + 1 + i] = sibling->keys[i];
    if (!child->leaf) {
      for (int i = 0; i <= sibling->count; ++i)
        child->children[child->count + 1 + i] = sibling->children[i];
    }
    child->count = child->count + 1 + sibling->count;
    for (int i = idx + 1; i < node->count; ++i)
      node->keys[i - 1] = node->keys[i];
    for (int i = idx + 2; i <= node->count; ++i)
      node->children[i - 1] = node->children[i];
    node->count -= 1;

    if (sibling) {
      sibling->leaf = sibling->leaf;
      for (int i = 0; i <= sibling->count; ++i) sibling->children[i] = nullptr;
      if (sibling->keys) delete[] sibling->keys;
      if (sibling->children) delete[] sibling->children;
      delete sibling;
      node->children[node->count + 1] = nullptr;
    }
  }

  void removeFromNode(Node<TK>* node, TK key) {
    if (!node) return;
    int idx = findKey(node, key);
    if (idx < node->count && node->keys[idx] == key) {
      if (node->leaf) {
        for (int i = idx + 1; i < node->count; ++i)
          node->keys[i - 1] = node->keys[i];
        node->count -= 1;
        n -= 1;
        return;
      } else {
        int minKeys = (int)ceil((double)M / 2.0) - 1;
        Node<TK>* predChild = node->children[idx];
        Node<TK>* succChild = node->children[idx + 1];
        if (predChild->count >= minKeys + 1) {
          TK pred = getPredecessor(node, idx);
          node->keys[idx] = pred;
          removeFromNode(predChild, pred);
        } else if (succChild->count >= minKeys + 1) {
          TK succ = getSuccessor(node, idx);
          node->keys[idx] = succ;
          removeFromNode(succChild, succ);
        } else {
          merge(node, idx);
          removeFromNode(node->children[idx], key);
        }
        return;
      }
    } else {
      if (node->leaf) {return;}
      bool flag = (idx == node->count);
      Node<TK>* child = node->children[idx];
      int minKeys = (int)ceil((double)M / 2.0) - 1;
      if (child->count < minKeys + 1) {
        fill(node, idx);
      }
      if (flag && idx > node->count) {
        removeFromNode(node->children[idx - 1], key);
      } else {
        removeFromNode(node->children[ (idx < node->count+1) ? idx : node->count ], key);
      }
    }
  }

  int computeHeight(Node<TK>* node) {
    if (!node) return 0;
    int h = 0;
    Node<TK>* cur = node;
    while (cur && !cur->leaf) {
      h++;
      cur = cur->children[0];
    }
    if (cur) return h + 1;
    return 0;
  }

  bool checkNode(Node<TK>* node, int depth, int& leafLevel) {
    if (!node) return true;
    for (int i = 1; i < node->count; ++i) {
      if (node->keys[i - 1] > node->keys[i]) return false;
    }
    if (!node->leaf) {
      for (int i = 0; i <= node->count; ++i) {
        if (!node->children[i]) return false;
      }
      for (int i = 0; i <= node->count; ++i) {
        if (!checkNode(node->children[i], depth + 1, leafLevel)) return false;
      }
    } else {
      if (leafLevel == -1) leafLevel = depth;
      else if (leafLevel != depth) return false;
    }
    int minKeys = (int)ceil((double)M / 2.0) - 1;
    if (node != root) {
      if (node->count < minKeys || node->count > M - 1) return false;
    } else {
      if (node->count < 1 || node->count > M - 1) return false;
    }
    return true;
  }

 public:
  BTree(int _M) : root(nullptr), M(_M), n(0) {}

  bool search(TK key){ //indica si se encuentra o no un elemento
    return searchNode(root, key);
  }

  void insert(TK key){ //inserta un elemento
    if (!root) {
      root = new Node<TK>(M);
      root->keys[0] = key;
      root->count = 1;
      root->leaf = true;
      n = 1;
      return;
    }
    if (search(key)) return;

    if (root->count == M - 1) {
      Node<TK>* s = new Node<TK>(M);
      s->leaf = false;
      s->children[0] = root;
      root = s;
      splitChild(s, 0);
      int i = 0;
      if (s->keys[0] < key) i = 1;
      insertNonFull(s->children[i], key);
    } else {
      insertNonFull(root, key);
    }
  }

  void remove(TK key){;//elimina un elemento
  if (!root) return;
    if (!search(key)) return;
    removeFromNode(root, key);
    if (root->count == 0) {
      Node<TK>* old = root;
      if (root->leaf) {
        root = nullptr;
        if (old) {
          if (old->keys) delete[] old->keys;
          if (old->children) delete[] old->children;
          delete old;
        }
      } else {
        root = root->children[0];
        if (old->keys) delete[] old->keys;
        if (old->children) delete[] old->children;
        delete old;
      }
    }
  }

  int height(){;//altura del arbol. Considerar altura 0 para arbol vacio
    if (!root) return 0;
    int levels = 0;
    Node<TK>* cur = root;
    while (cur) {
      levels++;
      if (cur->leaf) break;
      cur = cur->children[0];
    }
    return levels;
  }

  string toString(const string& sep){;  // recorrido inorder
    vector<TK> out;
    inorder(root, out);
    ostringstream oss;
    for (size_t i = 0; i < out.size(); ++i) {
      oss << out[i];
      if (i + 1 < out.size()) oss << sep;
    }
    return oss.str();
  }

  vector<TK> rangeSearch(TK begin, TK end){;
    if (!root) return out;
    function<void(Node<TK>*)> helper = [&](Node<TK>* node) {
      if (!node) return;
      int i = 0;
      while (i < node->count && node->keys[i] < begin) {
        if (!node->leaf) helper(node->children[i]);
        i++;
      }
      for ( ; i < node->count && node->keys[i] <= end; ++i) {
        if (!node->leaf) helper(node->children[i]);
        if (node->keys[i] >= begin && node->keys[i] <= end) out.push_back(node->keys[i]);
      }
      if (!node->leaf) helper(node->children[i]);
    };
    helper(root);
    return out;
  }

  TK minKey(){;  // minimo valor de la llave en el arbol
    if (!root) throw runtime_error("Tree is empty");
    Node<TK>* cur = root;
    while (!cur->leaf) cur = cur->children[0];
    return cur->keys[0];
  }

  TK maxKey(){;  // maximo valor de la llave en el arbol
    if (!root) throw runtime_error("Tree is empty");
    Node<TK>* cur = root;
    while (!cur->leaf) cur = cur->children[cur->count];
    return cur->keys[cur->count - 1];
  }

  void clear(){; // eliminar todos lo elementos del arbol
    if (root) {
      Node<TK>* tmp = root;
      root = nullptr;
      tmp->killSelf();
    }
    n = 0;
    root = nullptr;
  }

  int size(){return n;} // retorna el total de elementos insertados  
  
  static BTree* build_from_ordered_vector(const vector<TK>& elements, int order) {
    BTree* tree = new BTree(order);
    if (elements.empty()) return tree;
    int maxKeys = order - 1;
    vector<Node<TK>*> currentLevel;
    int i = 0;
    while (i < (int)elements.size()) {
      Node<TK>* leaf = new Node<TK>(order);
      leaf->leaf = true;
      int j = 0;
      while (j < maxKeys && i < (int)elements.size()) {
        leaf->keys[j++] = elements[i++];
      }
      leaf->count = j;
      currentLevel.push_back(leaf);
    }
    if (currentLevel.size() == 1) {
      tree->root = currentLevel[0];
      tree->n = elements.size();
      return tree;
    }
    while (currentLevel.size() > 1) {
      vector<Node<TK>*> parentLevel;
      int sz = currentLevel.size();
      int idx = 0;
      while (idx < sz) {
        Node<TK>* parent = new Node<TK>(order);
        parent->leaf = false;
        int childCount = 0;
        while (childCount < order && idx < sz) {
          parent->children[childCount] = currentLevel[idx++];
          childCount++;
        }
        for (int k = 0; k < childCount - 1; ++k) {
          Node<TK>* left = parent->children[k];
          parent->keys[k] = left->keys[left->count - 1];
        }
        parent->count = childCount - 1;
        parentLevel.push_back(parent);
      }
      currentLevel.swap(parentLevel);
    }
    tree->root = currentLevel[0];
    tree->n = elements.size();
    return tree;
  }

  static BTree* build_from_ordered_vector(const vector<TK>& elements) {
    return build_from_ordered_vector(elements, 5);
  }

  // Verifique las propiedades de un árbol B
  bool check_properties(){
    if (!root) return true;
    int leafLevel = -1;
    return checkNode(root, 0, leafLevel);
  }

  ~BTree(){clear();} // liberar memoria
};

#endif}