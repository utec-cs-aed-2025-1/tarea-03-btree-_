#ifndef NODE_H
#define NODE_H

#include <iostream>

using namespace std;

template <typename TK>
struct Node {
  TK* keys;
  Node** children;
  int count;
  bool leaf;
  int M;

  Node() : keys(nullptr), children(nullptr), count(0), leaf(true), M(0) {}

  Node(int _M) {
    M = _M;
    keys = new TK[M - 1];
    children = new Node<TK>*[M];
    for (int i = 0; i < M; ++i) children[i] = nullptr;
    count = 0;
    leaf = true;
  }

  void killSelf() {
    if (children != nullptr && M > 0) {
      for (int i = 0; i < M; ++i) {
        if (children[i]) {
          Node<TK>* ch = children[i];
          children[i] = nullptr;
          ch->killSelf();
        }
      }
    }

    if (keys) {
      delete[] keys;
      keys = nullptr;
    }
    if (children) {
      delete[] children;
      children = nullptr;
    }
    delete this;
  }
};

#endif
