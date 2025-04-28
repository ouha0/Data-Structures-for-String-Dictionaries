# Research Project: Cache-Aware Data Structures for Managing String Dictionaries

---

## Abstract

This project experimentally investigates the performance of various data structures for managing large, in-memory string dictionaries, with a focus on 
cache efficiency on modern commodity machines. Traditional analysis often overlooks cache effects, especially for variable-length strings.
We surveyed Binary Search Trees (BSTs), B-trees, B+-trees, array hash tables, and chained hash tables (both with and without the Move-to-Front heuristic).
Experiments were conducted using three distinct datasets: a unique word set, skewed data parsed from Wikipedia, and skewed data with common English words removed. 
Our findings show significant performance differences driven by cache behavior and data distribution. Notably, array hash tables consistently demonstrated superior 
performance across all datasets. 
BSTs outperformed B-trees on skewed data, while B-trees scaled better on uniform data. Interestingly,
the MTF heuristic did not yield the performance benefits observed in older research, suggesting changes in hardware or datasets may have altered its effectiveness.

**Keywords:** Array-hash, Reference Locality, Compact data layout, Cache, Cache-aware data structures, B-trees, String Dictionaries, Performance Evaluation

---

## Motivation and Background

Traditional data structure analysis often assumes uniform memory access costs (RAM model). However, the significant latency gap between CPU speed and memory access (the "memory wall") 
makes cache performance critical on modern hardware. This project revisits fundamental data structures for string dictionary management, 
considering cache-conscious principles like spatial and temporal locality. Much prior work focused on fixed-size keys or older hardware architectures. This research re-evaluates these
structures for variable-length strings on contemporary machines, focusing on compact, array-based implementations where possible.

---

## Data Structures Implemented

This repository contains C implementations of the following data structures, designed with cache-awareness principles (e.g., compact layouts using byte arrays):

* **Binary Search Tree (BST)
* **B-Tree:**
* **B+ Tree:**
* **Chained Hash Table (Linked Hash):** 
* **Chained Hash Table w/ Move-to-Front (MTF Linked Hash):**
* **Array Hash Table:**
* **Array Hash Table w/ Move-to-Front (MTF Array Hash):**

---

