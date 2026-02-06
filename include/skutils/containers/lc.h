#ifndef SHUAIKAI_DATDSTRUCTURE_LC_H
#define SHUAIKAI_DATDSTRUCTURE_LC_H

#include <functional>
#include <span>
#include <sstream>
#include <string>

#include "skutils/config.h"

namespace sk::utils::dts::lc {

struct ListNode {
  int val;
  ListNode *next;

  explicit ListNode(int v) : val(v), next(nullptr) {}

  ListNode(int v, ListNode *n) : val(v), next(n) {}

  ~ListNode() { delete next; }

  std::string toString();
};

struct TreeNode {
  int val;
  TreeNode *left;
  TreeNode *right;

  explicit TreeNode(int x = 0, TreeNode *left = nullptr, TreeNode *right = nullptr)
    : val(x), left(left), right(right) {}

  ~TreeNode() {
    delete left;
    delete right;
  }

  std::string toString();
};

/// MARK: tree/List tools
inline std::string treeToString(TreeNode *root, const std::string &sep) {
  std::string ret;
  // inorder
  std::stringstream ss;
  TreeNode *p = root;
  ss << "(In)[";
  std::function<void(sk::utils::dts::lc::TreeNode *)> inorder = [&](TreeNode *node) {
    if (node != nullptr) {
      inorder(node->left);
      ss << node->val << sep;
      inorder(node->right);
    }
  };
  inorder(p);
  ret = ss.str().substr(0, ss.str().length() - sep.length()) + "]";
  // preorder
  std::stringstream ss2;
  ss2 << "(Pre)[";
  std::function<void(TreeNode *)> preorder = [&](TreeNode *node) {
    if (node != nullptr) {
      ss2 << node->val << sep;
      preorder(node->left);
      preorder(node->right);
    }
  };
  TreeNode *q = root;
  preorder(q);
  std::string ret2 = ss2.str().substr(0, ss2.str().length() - sep.length()) + "]";
  return ret2 + sep + ret;
}

inline std::string listToString(ListNode *list, const std::string &sep) {
  std::stringstream ss;
  ListNode *p = list;
  ss << "[";
  while (p != nullptr) {
    ss << p->val << sep;
    p = p->next;
  }
  return ss.str().substr(0, ss.str().length() - sep.length()) + "]";
}

inline std::string TreeNode::toString() {
  return treeToString(this, ELEM_SEP);
}

inline std::string ListNode::toString() {
  return listToString(this, ELEM_SEP);
}

inline ListNode *vector2List(std::vector<int> &&data) {
  int len = data.size();
  auto *head = new ListNode(data[0]);
  auto *tail = head;
  for (int i = 1; i < len; i++) {
    tail->next = new ListNode(data[i]);
    tail = tail->next;
  }
  return head;
}

inline ListNode *vector2List(std::vector<int> &data) {
  return vector2List(std::move(data));
}

inline void reverseList(ListNode **head) {
  ListNode *p = *head;
  ListNode *q = p->next;
  if (q == nullptr) {
    return;
  }
  p->next = nullptr;
  while (q->next != nullptr) {
    ListNode *r = q->next;
    q->next = p;
    p = q;
    q = r;
  }
  q->next = p;
  *head = q;
}

}  // namespace sk::utils::dts::lc

#endif  // SHUAIKAI_DATDSTRUCTURE_LC_H
