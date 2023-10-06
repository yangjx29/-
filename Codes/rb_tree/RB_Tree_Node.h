#ifndef RB_Tree_Node_H
#define RB_Tree_Node_H
/*
    1.节点是红色或者黑色的
    2.根是黑色的
    3.叶子节点都是黑色的。这里的叶子节点指的是最底层的空节点(外部节点)，
    4.红色节点的子节点都是黑色、红色节点的父节点都是黑色
        即从根节点到叶子节点的所有路径上不能有2个连续的红色节点
    5.从任意节点到叶子节点的所有路径都包含相同数目的黑色节点(注意还要包括不存在的空节点也要算黑色)

    效率：
        红黑树的查找、插入和删除效率都是O(logN)

    红黑树与平衡二叉树的比较：
        1.AVL树的时间复杂度虽然优于红黑树，但是对于现在的计算机，cpu太快，可以忽略性能差异
        2.红黑树的插入删除比AVL树更便于控制操作
        3.红黑树整体性能略优于AVL树（红黑树旋转情况少于AVL树）
*/

template <class T>
class RB_Tree_Node {
public:
    RB_Tree_Node(T data_in);
    ~RB_Tree_Node();
    RB_Tree_Node* Right_child;
    RB_Tree_Node* Left_child;
    RB_Tree_Node* Father_Node;
    T data;
    int color_tag;
};

#endif