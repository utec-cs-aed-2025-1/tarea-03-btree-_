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

 public:
  BTree(int _M) : root(nullptr), M(_M) {}

  bool search(TK key);//indica si se encuentra o no un elemento
  void insert(TK key);//inserta un elemento
  void remove(TK key);//elimina un elemento
  int height();//altura del arbol. Considerar altura 0 para arbol vacio
  string toString(const string& sep);  // recorrido inorder
  vector<TK> rangeSearch(TK begin, TK end);

  TK minKey();  // minimo valor de la llave en el arbol
  TK maxKey();  // maximo valor de la llave en el arbol
  void clear(); // eliminar todos lo elementos del arbol
  int size(); // retorna el total de elementos insertados  
  
  // Construya un árbol B a partir de un vector de elementos ordenados
  static BTree* build_from_ordered_vector(vector<T> elements);
  // Verifique las propiedades de un árbol B
  bool check_properties();

  ~BTree();     // liberar memoria
};

#endif
