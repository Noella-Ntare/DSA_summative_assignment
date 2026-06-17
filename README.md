# Data Structures Summative Project

Five C implementations covering core data structures and algorithms.

---

## Q1 – Emergency Dispatch Incident Tracker (Doubly Linked List)

**Compile:**
```
cd q1
make
```
**Run:**
```
./incident_tracker
```
**Commands:** `f` forward, `b` backward, `l` live monitoring, `s` stop, `d` delete all, `q` save & quit

Stores up to 25 incidents. When full, the oldest is automatically dropped. Live monitoring runs on a background thread and inserts new incidents every 3 seconds while you navigate history.

---

## Q2 – Maintenance Procedure Validator (Binary Search Tree)

**Compile:**
```
cd q2
make
```
**Run:**
```
./validator
```
Loads procedures from `procedures.txt` into a BST. Exact matches are approved. Close matches (Levenshtein distance ≤ 4) trigger a suggestion. Unknown entries are rejected and logged to `audit.log`.

---

## Q3 – Airline Route Analyzer (Directed Graph)

**Compile:**
```
cd q3
make
```
**Run:**
```
./route_analyzer
```
**Commands:**
- `query <CODE>` — show incoming and outgoing routes
- `matrix` — print adjacency matrix
- `list` — list all airports
- `add_airport <CODE>`, `add_route <FROM> <TO>`, `del_route <FROM> <TO>`, `del_airport <CODE>`

---

## Q4 – Campus Delivery Robot Navigation (Dijkstra's Algorithm)

**Compile:**
```
cd q4
make
```
**Run:**
```
./robot_nav
```
Enter any campus building name and the program finds the shortest weighted path to the Dormitory using Dijkstra's algorithm. Invalid building names are handled gracefully.

---

## Q5 – Telemetry Data Compression (Huffman Coding)

**Compile:**
```
cd q5
make
```
**Run:**
```
./huffman telemetry.txt
```
Compresses the input file to `telemetry.huff`, then decompresses back to `telemetry_restored.txt`, and verifies the restored file matches the original byte-for-byte.

---

## Project Structure

```
q1/  incident_tracker.c  Makefile
q2/  validator.c         Makefile  procedures.txt
q3/  route_analyzer.c   Makefile
q4/  robot_nav.c        Makefile
q5/  huffman.c          Makefile  telemetry.txt
```
