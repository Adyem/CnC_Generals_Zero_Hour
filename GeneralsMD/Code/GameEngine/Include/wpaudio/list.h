#pragma once

#include "wpaudio/altypes.h"

#include <cstdint>

using Priority = std::intptr_t;

struct ListNode
{
    ListNode* prev = nullptr;
    ListNode* next = nullptr;
    Priority pri = 0;
};

using ListHead = ListNode;

inline bool ListNodeIsHead(const ListNode* node)
{
    return node != nullptr && node->pri == reinterpret_cast<Priority>(node);
}

inline bool ListNodeInList(const ListNode* node)
{
    return node != nullptr && (node->next != node || node->prev != node);
}

void ListInit(ListHead* head);
void ListNodeInit(ListNode* node);
int ListAddNodeSortAscending(ListHead* head, ListNode* newNode);
void ListAddNode(ListHead* head, ListNode* newNode);
void ListAddNodeAfter(ListHead* head, ListNode* newNode);
void ListMerge(ListHead* from, ListHead* to);
int ListCountItems(ListHead* head);
ListNode* ListFirstItem(ListHead* head);
ListNode* ListNextItem(ListNode* node);
ListNode* ListGetItem(ListHead* head, int number);
void ListNodeInsert(ListNode* node, ListNode* newNode);
void ListNodeAppend(ListNode* node, ListNode* newNode);
void ListNodeRemove(ListNode* node);
ListNode* ListNodeNext(ListNode* node);
ListNode* ListNodePrev(ListNode* node);
ListNode* ListNodeLoopNext(ListNode* node);
ListNode* ListNodeLoopPrev(ListNode* node);

inline void ListAddToTail(ListHead* head, ListNode* node)
{
    if (head != nullptr && node != nullptr)
    {
        ListNodeInsert(reinterpret_cast<ListNode*>(head), node);
    }
}

