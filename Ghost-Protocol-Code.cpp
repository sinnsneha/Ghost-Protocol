#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <windows.h>

using namespace std;

//   COLOURS
string RESET   = "\033[0m";
string RED     = "\033[31m";
string GREEN   = "\033[32m";
string YELLOW  = "\033[33m";
string CYAN    = "\033[36m";
string MAGENTA = "\033[35m";
string BRED    = "\033[1;31m";
string BGREEN  = "\033[1;32m";
string BYELLOW = "\033[1;33m";
string BCYAN   = "\033[1;36m";
string BMAGENTA= "\033[1;35m";
string BWHITE  = "\033[1;97m";

//   GAME STATE
struct State
{
    string name;
    int  entryPointID;
    int  suspectCount;
    int  insiderNode;
    bool alarmDisabled;
    int  score[5];
    int  hintsUsed[5];
    bool gamePassed[5];
    int  nim_pile0, nim_pile1, nim_pile2;
    int  bridgeU, bridgeV;
    int  escapeHops;
};

State gs;

void initState()
{
    gs.entryPointID  = 0;
    gs.suspectCount  = 0;
    gs.insiderNode   = 0;
    gs.alarmDisabled = true;
    for (int i = 0; i < 5; i++)
    {
        gs.score[i]      = 0;
        gs.hintsUsed[i]  = 0;
        gs.gamePassed[i] = false;
    }
    gs.nim_pile0  = 0;
    gs.nim_pile1  = 0;
    gs.nim_pile2  = 0;
    gs.bridgeU    = -1;
    gs.bridgeV    = -1;
    gs.escapeHops = 0;
}

//   UTILITY
void clearScreen() {  }

void pressEnter()
{
    cout << BMAGENTA << "\n  [Press ENTER to continue]" << RESET;
    cin.ignore(100000, '\n');
}

void printLine()
{
    cout << BCYAN << "  ============================================================\n" << RESET;
}

void printBanner(string text)
{
    printLine();
    int pad = (58 - (int)text.size()) / 2;
    if (pad < 0) pad = 0;
    cout << BCYAN << "  ";
    for (int i = 0; i < pad; i++) cout << " ";
    cout << text << "\n";
    printLine();
    cout << "\n";
}

int inputInt(string prompt)
{
    while (true)
    {
        cout << BYELLOW << prompt << RESET;
        string s;
        getline(cin, s);
        if (s.empty()) { cout << RED << "  Enter a number.\n" << RESET; continue; }
        bool ok = true;
        int start = 0;
        if (s[0] == '-') start = 1;
        for (int i = start; i < (int)s.size(); i++)
            if (s[i] < '0' || s[i] > '9') { ok = false; break; }
        if (!ok) { cout << RED << "  Enter a number.\n" << RESET; continue; }
        int val = 0, sign = 1, i = 0;
        if (s[0] == '-') { sign = -1; i = 1; }
        for (; i < (int)s.size(); i++) val = val * 10 + (s[i] - '0');
        return sign * val;
    }
}

long long inputLL(string prompt)
{
    while (true)
    {
        cout << BYELLOW << prompt << RESET;
        string s;
        getline(cin, s);
        if (s.empty()) { cout << RED << "  Enter a number.\n" << RESET; continue; }
        bool ok = true;
        int start = 0;
        if (s[0] == '-') start = 1;
        for (int i = start; i < (int)s.size(); i++)
            if (s[i] < '0' || s[i] > '9') { ok = false; break; }
        if (!ok) { cout << RED << "  Enter a number.\n" << RESET; continue; }
        long long val = 0; int sign = 1, i = 0;
        if (s[0] == '-') { sign = -1; i = 1; }
        for (; i < (int)s.size(); i++) val = val * 10 + (s[i] - '0');
        return (long long)sign * val;
    }
}

bool askHint(int gameIdx)
{
    cout << MAGENTA << "\n  Press H for a hint (-5 points) or ENTER to skip: " << RESET;
    string s;
    getline(cin, s);
    if (s == "H" || s == "h")
    {
        gs.hintsUsed[gameIdx]++;
        gs.score[gameIdx] -= 5;
        if (gs.score[gameIdx] < 0) gs.score[gameIdx] = 0;
        return true;
    }
    return false;
}

//   ALGORITHMS

// --- Sliding Window -----------------------------------------
int longestZeroWindow(int arr[], int n, int &bestLen)
{
    int left = 0, best = 0, bestStart = 0, sum = 0;
    for (int right = 0; right < n; right++)
    {
        sum += arr[right];
        while (sum > 0) { sum -= arr[left]; left++; }
        if (right - left + 1 > best)
        {
            best      = right - left + 1;
            bestStart = left;
        }
    }
    bestLen = best;
    return bestStart;
}

// --- Nim XOR ------------------------------------------------
int nimXor(int p0, int p1, int p2) { return p0 ^ p1 ^ p2; }

void nimAIMove(int piles[], int &idx, int &amount)
{
    int x = piles[0] ^ piles[1] ^ piles[2];
    for (int i = 0; i < 3; i++)
    {
        int target = piles[i] ^ x;
        if (target < piles[i]) { idx = i; amount = piles[i] - target; return; }
    }
    idx = 0;
    for (int i = 1; i < 3; i++) if (piles[i] > piles[idx]) idx = i;
    amount = 1;
}

// --- Tarjan Bridge Detection --------------------------------
const int MAXN = 8;
const int MAXE = 16;

int adjU[MAXE], adjV[MAXE];
int edgeCnt;
int disc_arr[MAXN], low_arr[MAXN], timer_val;
bool visited_arr[MAXN];
bool isBridge[MAXE];

void initGraph(int n)
{
    edgeCnt   = 0;
    timer_val = 0;
    for (int i = 0; i < n; i++)
    {
        disc_arr[i]    = -1;
        low_arr[i]     = 0;
        visited_arr[i] = false;
    }
    for (int i = 0; i < MAXE; i++) isBridge[i] = false;
}

void addEdge(int u, int v) { adjU[edgeCnt] = u; adjV[edgeCnt] = v; edgeCnt++; }

void tarjanDFS(int u, int parentEdge)
{
    disc_arr[u] = low_arr[u] = timer_val++;
    visited_arr[u] = true;
    for (int i = 0; i < edgeCnt; i++)
    {
        int v = -1;
        if      (adjU[i] == u) v = adjV[i];
        else if (adjV[i] == u) v = adjU[i];
        if (v == -1 || i == parentEdge) continue;
        if (!visited_arr[v])
        {
            tarjanDFS(v, i);
            if (low_arr[v] < low_arr[u]) low_arr[u] = low_arr[v];
            if (low_arr[v] > disc_arr[u]) isBridge[i] = true;
        }
        else if (disc_arr[v] < low_arr[u]) low_arr[u] = disc_arr[v];
    }
}

// --- DSU ----------------------------------------------------
int dsu_parent[MAXN], dsu_rank[MAXN];

void dsuInit(int n)
{
    for (int i = 0; i < n; i++) { dsu_parent[i] = i; dsu_rank[i] = 0; }
}

int dsuFind(int x)
{
    if (dsu_parent[x] != x) dsu_parent[x] = dsuFind(dsu_parent[x]);
    return dsu_parent[x];
}

void dsuUnite(int a, int b)
{
    int ra = dsuFind(a), rb = dsuFind(b);
    if (ra == rb) return;
    if (dsu_rank[ra] < dsu_rank[rb]) { int t = ra; ra = rb; rb = t; }
    dsu_parent[rb] = ra;
    if (dsu_rank[ra] == dsu_rank[rb]) dsu_rank[ra]++;
}

// --- Graph for Game 5 ------------------
vector<int> depAdj[8];
vector<int> escAdj[8];
int indeg_arr[8];

void clearGraph5()
{
    for (int i = 0; i < 8; i++) { depAdj[i].clear(); escAdj[i].clear(); indeg_arr[i] = 0; }
}

// --- Topological Sort (Kahn's algorithm) --------------------
vector<int> topoSort(int n)
{
    int deg[8];
    for (int i = 0; i < n; i++) deg[i] = indeg_arr[i];

    queue<int> q;
    for (int i = 0; i < n; i++) if (deg[i] == 0) q.push(i);

    vector<int> order;
    while (!q.empty())
    {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (int j = 0; j < (int)depAdj[u].size(); j++)
        {
            int v = depAdj[u][j];
            deg[v]--;
            if (deg[v] == 0) q.push(v);
        }
    }
    return order;
}

// --- BFS Shortest Path --------------------------------------
int bfsHops(int n, int src, int dst)
{
    int dist[8];
    for (int i = 0; i < n; i++) dist[i] = -1;
    queue<int> q;
    q.push(src); dist[src] = 0;
    while (!q.empty())
    {
        int u = q.front(); q.pop();
        if (u == dst) return dist[u];
        for (int j = 0; j < (int)escAdj[u].size(); j++)
        {
            int v = escAdj[u][j];
            if (dist[v] == -1) { dist[v] = dist[u] + 1; q.push(v); }
        }
    }
    return -1;
}

//   GAME 1 - SONAR SWEEP   (Sliding Window)
void game1()
{
    clearScreen();
    printBanner("GAME 1 - SONAR SWEEP");

    cout << "  MegaCorp's scanner sweeps the network in pulses.\n";
    cout << "  Find the longest safe gap to slip through undetected.\n";
    cout << "  The start index of that gap becomes your ENTRY POINT ID.\n\n";

    int n = 12;
    int patrol[12];
    srand((unsigned int)time(0));

    for (int i = 0; i < n; i++) patrol[i] = rand() % 3;
    int gapStart = rand() % 8;
    patrol[gapStart]   = 0;
    patrol[gapStart+1] = 0;
    patrol[gapStart+2] = 0;
    if (gapStart > 0)   patrol[gapStart-1] = 1;
    if (gapStart+3 < n) patrol[gapStart+3] = 1;

    cout << YELLOW << "  SIGNAL READINGS:\n";
    cout << "  Index: ";
    for (int i = 0; i < n; i++) cout << "  " << i;
    cout << "\n  Value: ";
    for (int i = 0; i < n; i++)
    {
        if (patrol[i] == 0) cout << BGREEN << "  0" << YELLOW;
        else                 cout << BRED   << "  " << patrol[i] << YELLOW;
    }
    cout << "\n" << RESET;
    cout << CYAN << "  (Green = safe, Red = guard active)\n\n" << RESET;

    cout << BMAGENTA << "  HOW SLIDING WINDOW WORKS:\n" << RESET;
    cout << "  Keep two pointers: left and right.\n";
    cout << "  Move right forward each step.\n";
    cout << "  If a guard appears, move left forward until the guard is gone.\n";
    cout << "  Track the longest all-zero stretch seen so far.\n\n";

    int bestLen = 0;
    int correctStart = longestZeroWindow(patrol, n, bestLen);

    cout << "  Longest safe window found: " << BGREEN << bestLen << RESET << " slots\n\n";

    cout << "  (Enter H before answering for a hint)\n";
    if (askHint(0))
    {
        cout << BMAGENTA << "\n  HINT - Sliding Window\n" << RESET;
        cout << "  Scan left to right. Whenever you hit a non-zero value\n";
        cout << "  your current window ends. Start fresh from the next slot.\n";
        cout << "  Keep track of the longest all-zero window you found.\n\n";
    }

    cout << BYELLOW << "\n  PUZZLE: Where does the longest safe window START?\n" << RESET;
    cout << "  (Enter an index from 0 to " << n-1 << ")\n\n";

    gs.score[0] = 20;
    int attempts = 0;

    while (attempts < 3)
    {
        int ans = inputInt("  Your answer: ");
        attempts++;

        bool valid = true;
        if (ans < 0 || ans + bestLen - 1 >= n) valid = false;
        else
            for (int i = ans; i < ans + bestLen; i++)
                if (patrol[i] != 0) { valid = false; break; }

        if (valid)
        {
            cout << BGREEN << "\n  CORRECT! Slipping through slots "
                 << ans << " to " << ans + bestLen - 1 << ".\n" << RESET;
            gs.gamePassed[0] = true;
            gs.entryPointID  = ans % 8;
            break;
        }
        else
        {
            gs.score[0] -= 5;
            if (gs.score[0] < 0) gs.score[0] = 0;
            if (attempts < 3)
                cout << BRED << "  Wrong. Guards spotted movement. -5 points. Try again.\n" << RESET;
            else
            {
                cout << BRED << "\n  Failed. Correct start was index " << correctStart << ".\n" << RESET;
                cout << "  Using correct value to keep the chain alive.\n";
                gs.entryPointID = correctStart % 8;
                gs.score[0]     = 0;
            }
        }
    }

    cout << "\n  CHAIN: Entry Point ID = " << BGREEN << gs.entryPointID
         << RESET << "  (becomes XOR key in Game 2)\n";
    pressEnter();
}

//   GAME 2 - VAULT BREAKER   (XOR + Bit Manipulation)
void game2()
{
    clearScreen();
    printBanner("GAME 2 - VAULT BREAKER");

    cout << "  The vault encrypts data using your Entry Point ID as the XOR key.\n";
    cout << "  Decrypt the suspect list to count how many suspects are hiding.\n\n";

    int key = gs.entryPointID;
    if (key == 0) key = 3;

    cout << "  ENTRY POINT ID (from Game 1): " << BGREEN << key << RESET << "\n\n";

    srand((unsigned int)time(0) + 1);
    int original[4], encrypted[4];
    for (int i = 0; i < 4; i++)
    {
        original[i]  = (rand() % 200) + 20;
        encrypted[i] = original[i] ^ key;
    }

    cout << YELLOW << "  ENCRYPTED SUSPECT DATA:\n";
    cout << "  Index:     0      1      2      3\n";
    cout << "  Value: ";
    for (int i = 0; i < 4; i++) cout << "  " << encrypted[i];
    cout << "\n\n" << RESET;

    cout << BMAGENTA << "  HOW XOR DECRYPTION WORKS:\n" << RESET;
    cout << "  encrypted_value XOR key = original_value\n";
    cout << "  This works because:  (A XOR K) XOR K = A\n";
    cout << "  XOR is its own inverse.\n\n";

    cout << "  EXAMPLE:\n";
    cout << "  encrypted[0] = " << encrypted[0] << "\n";
    cout << "  key          = " << key << "\n";
    cout << "  decrypted[0] = " << encrypted[0] << " XOR " << key
         << " = " << BGREEN << (encrypted[0] ^ key) << RESET << "\n\n";

    cout << BMAGENTA << "  BIT MANIPULATION CHECK:\n" << RESET;
    int checkVal = original[0];
    int bitCount = 0, tmp = checkVal;
    while (tmp > 0) { bitCount += (tmp & 1); tmp >>= 1; }
    cout << "  Decrypted[0] = " << checkVal << " in binary: ";
    for (int b = 7; b >= 0; b--) cout << ((checkVal >> b) & 1);
    cout << "\n  Number of 1-bits = " << BGREEN << bitCount << RESET << "\n\n";

    cout << "  (Enter H for a hint)\n";
    if (askHint(1))
    {
        cout << BMAGENTA << "\n  HINT - XOR Decryption\n" << RESET;
        cout << "  To decrypt: take each encrypted value and XOR it with the key.\n";
        cout << "  Example: if encrypted = 142 and key = 3, then 142 XOR 3 = "
             << (142 ^ 3) << "\n\n";
    }

    cout << BYELLOW << "\n  PUZZLE: What is decrypted[2]?\n" << RESET;
    cout << "  (encrypted[2] = " << encrypted[2] << ", key = " << key << ")\n\n";

    gs.score[1] = 20;
    int correct2 = original[2];
    int attempts = 0;

    while (attempts < 3)
    {
        int ans = inputInt("  Your answer: ");
        attempts++;

        if (ans == correct2)
        {
            cout << BGREEN << "\n  CORRECT! Suspect list decrypted.\n" << RESET;
            gs.gamePassed[1] = true;
            int combined = original[0] ^ original[1] ^ original[2] ^ original[3];
            int cnt = 0;
            while (combined > 0) { cnt += (combined & 1); combined >>= 1; }
            gs.suspectCount = cnt;
            if (gs.suspectCount < 2) gs.suspectCount = 2;
            if (gs.suspectCount > 7) gs.suspectCount = 7;
            break;
        }
        else
        {
            gs.score[1] -= 5;
            if (gs.score[1] < 0) gs.score[1] = 0;
            if (attempts < 3)
                cout << BRED << "  Wrong. Decryption failed. -5 points. Try again.\n" << RESET;
            else
            {
                cout << BRED << "\n  Failed. Correct answer was: " << correct2 << "\n" << RESET;
                gs.suspectCount = 4;
                gs.score[1]     = 0;
            }
        }
    }

    cout << "\n  CHAIN: Suspects found = " << BGREEN << gs.suspectCount
         << RESET << "  (sets Nim pile sizes in Game 3)\n";
    pressEnter();
}

//   GAME 3 - SHADOW NIM   (Sprague-Grundy / Nim XOR)
void game3()
{
    clearScreen();
    printBanner("GAME 3 - SHADOW NIM");

    cout << "  An AI guardian blocks the server core.\n";
    cout << "  The number of suspects you found sets the pile sizes.\n";
    cout << "  Beat the AI to reveal the insider node ID.\n\n";

    int sc = gs.suspectCount;
    int piles[3];
    piles[0] = sc;
    piles[1] = sc + 2;
    piles[2] = sc + 1;

    gs.nim_pile0 = piles[0];
    gs.nim_pile1 = piles[1];
    gs.nim_pile2 = piles[2];

    cout << "  Suspects found: " << BGREEN << sc << RESET
         << "  ->  Pile sizes: ["
         << piles[0] << ", " << piles[1] << ", " << piles[2] << "]\n\n";

    cout << BMAGENTA << "  HOW NIM WORKS:\n" << RESET;
    cout << "  XOR all pile sizes together.\n";
    cout << "  If XOR = 0  -> LOSING  (any move hands advantage to opponent).\n";
    cout << "  If XOR != 0 -> WINNING (make a move so XOR becomes 0).\n";
    cout << "  Winning move: find pile where (pile XOR total_xor) < pile.\n";
    cout << "  Reduce that pile to (pile XOR total_xor).\n\n";

    cout << "  (Enter H for a hint)\n";
    if (askHint(2))
    {
        cout << BMAGENTA << "\n  HINT - Nim Strategy\n" << RESET;
        int x = piles[0] ^ piles[1] ^ piles[2];
        cout << "  Current XOR = " << piles[0] << " ^ " << piles[1]
             << " ^ " << piles[2] << " = " << x << "\n";
        if (x != 0)
        {
            cout << "  XOR != 0 so you are in a WINNING position.\n";
            for (int i = 0; i < 3; i++)
            {
                int target = piles[i] ^ x;
                if (target < piles[i])
                {
                    cout << "  Winning move: take " << (piles[i] - target)
                         << " from pile " << i << " (reduces it to " << target << ")\n";
                    break;
                }
            }
        }
        else cout << "  XOR = 0 so you are in a LOSING position. Survive as long as possible.\n";
        cout << "\n";
    }

    gs.score[2] = 20;
    int roundNum = 0;
    bool gameOver = false, playerWon = false;
    int lastPlayerPile = 0;

    while (!gameOver)
    {
        bool allZero = (piles[0] == 0 && piles[1] == 0 && piles[2] == 0);
        if (allZero)
        {
            playerWon = (roundNum % 2 == 1);
            gameOver  = true;
            break;
        }

        cout << CYAN << "\n  Piles: [pile0=" << piles[0]
             << "  pile1=" << piles[1] << "  pile2=" << piles[2] << "]\n" << RESET;

        int x = nimXor(piles[0], piles[1], piles[2]);
        cout << "  XOR = " << x;
        if (x != 0) cout << "  (winning position)\n";
        else        cout << "  (losing position)\n";

        if (roundNum % 2 == 0)
        {
            cout << BYELLOW << "  YOUR TURN\n" << RESET;
            int pi = inputInt("  Choose pile (0, 1, 2): ");
            int ta = inputInt("  Take how many: ");

            if (pi < 0 || pi > 2)
            { cout << RED << "  Invalid pile. Turn skipped.\n" << RESET; roundNum++; continue; }
            if (ta < 1 || ta > piles[pi])
            { cout << RED << "  Invalid amount. Turn skipped.\n" << RESET; roundNum++; continue; }

            lastPlayerPile = pi;
            piles[pi] -= ta;
        }
        else
        {
            cout << MAGENTA << "  AI thinking...\n" << RESET;
            int idx = 0, amt = 0;
            nimAIMove(piles, idx, amt);
            cout << MAGENTA << "  AI takes " << amt << " from pile" << idx << "\n" << RESET;
            piles[idx] -= amt;
        }

        allZero = (piles[0] == 0 && piles[1] == 0 && piles[2] == 0);
        if (allZero)
        {
            playerWon = (roundNum % 2 == 0);
            gameOver  = true;
        }
        roundNum++;
    }

    if (playerWon)
    {
        cout << BGREEN << "\n  YOU WIN! AI guardian steps aside.\n" << RESET;
        gs.gamePassed[2] = true;
        gs.insiderNode   = lastPlayerPile;
    }
    else
    {
        cout << BRED << "\n  AI wins. You force the door. Score penalty.\n" << RESET;
        gs.score[2]    = 5;
        gs.insiderNode = 2;
    }

    cout << "\n  CHAIN: Insider Node = " << BGREEN << gs.insiderNode
         << RESET << "  (camera cluster to isolate in Game 4)\n";
    pressEnter();
}

//   GAME 4 - CITY BLACKOUT   (Tarjan Bridges + DSU)
void game4()
{
    clearScreen();
    printBanner("GAME 4 - CITY BLACKOUT");

    cout << "  The insider's camera cluster is node " << BGREEN << gs.insiderNode << RESET << ".\n";
    cout << "  Find the bridge edge that connects it to the rest of the network.\n";
    cout << "  Cut it and that entire cluster goes dark.\n\n";

    int n = 6;
    initGraph(n);
    addEdge(0, 1);
    addEdge(1, 2);
    addEdge(2, 3);
    addEdge(3, 4);
    addEdge(4, 5);

    tarjanDFS(0, -1);

    dsuInit(n);
    for (int i = 0; i < edgeCnt; i++)
        if (!isBridge[i]) dsuUnite(adjU[i], adjV[i]);

    cout << YELLOW << "  SURVEILLANCE NETWORK:\n";
    cout << "  Nodes: 0  1  2  3  4  5\n";
    cout << "  Edges: 0-1  1-2  2-0  2-3  3-4  4-5\n\n" << RESET;

    cout << BCYAN << "  ASCII MAP OF THE NETWORK:\n\n" << RESET;
    cout << "      [0]----[1]\n";
    cout << "        \\   /\n";
    cout << "         \\ /\n";
    cout << "         [2]   <- triangle (0-1, 1-2, 2-0): NOT bridges\n";
    cout << BRED << "          |  <-- BRIDGE (2-3)\n" << RESET;
    cout << "         [3]\n";
    cout << BRED << "          |  <-- BRIDGE (3-4)\n" << RESET;
    cout << "         [4]\n";
    cout << BRED << "          |  <-- BRIDGE (4-5)\n" << RESET;
    cout << "         [5]\n\n";

    cout << "  Nodes 0, 1, 2 form a triangle (cycle).\n";
    cout << "  Edges 2-3, 3-4, 4-5 are single links (bridges).\n\n";

    cout << BMAGENTA << "  HOW TARJAN BRIDGE DETECTION WORKS:\n" << RESET;
    cout << "  Run DFS and track discovery time (disc) for each node.\n";
    cout << "  Also track low[v] = earliest node reachable from v's subtree.\n";
    cout << "  If low[v] > disc[u], the edge u-v is a bridge.\n";
    cout << "  (Means no back-edge from v's subtree reaches u or above)\n\n";

    int eu[6] = {0, 1, 2, 2, 3, 4};
    int ev[6] = {1, 2, 0, 3, 4, 5};

    cout << "  BRIDGE ANALYSIS RESULTS:\n";
    for (int i = 0; i < edgeCnt; i++)
    {
        if (isBridge[i])
            cout << BGREEN << "  Edge " << eu[i] << "-" << ev[i] << "  --> BRIDGE\n" << RESET;
        else
            cout << "  Edge " << eu[i] << "-" << ev[i] << "  --> not a bridge\n";
    }

    cout << "\n  DSU CLUSTER CHECK:\n";
    cout << "  Nodes 0,1,2 are in the same cluster (connected by non-bridges).\n";
    cout << "  Nodes 3, 4, 5 are each isolated by bridges.\n\n";

    cout << "  (Enter H for a hint)\n";
    if (askHint(3))
    {
        cout << BMAGENTA << "\n  HINT - Finding Bridges\n" << RESET;
        cout << "  A bridge is an edge that if removed splits the graph.\n";
        cout << "  Triangle edges (0-1, 1-2, 2-0) are NOT bridges.\n";
        cout << "  Single link edges (2-3, 3-4, 4-5) ARE bridges.\n";
        cout << "  Insider node = " << gs.insiderNode
             << ". Enter a bridge edge that reaches it.\n\n";
    }

    cout << BYELLOW << "\n  PUZZLE: Enter a bridge edge to cut.\n" << RESET;
    cout << "  Target: isolate node " << gs.insiderNode << " from the main network.\n\n";

    gs.score[3] = 20;
    int attempts = 0;

    while (attempts < 3)
    {
        int u = inputInt("  Node 1: ");
        int v = inputInt("  Node 2: ");
        attempts++;

        bool found = false;
        for (int i = 0; i < edgeCnt; i++)
        {
            if (isBridge[i])
            {
                if ((eu[i] == u && ev[i] == v) || (eu[i] == v && ev[i] == u))
                {
                    found = true;
                    gs.bridgeU = u;
                    gs.bridgeV = v;
                    break;
                }
            }
        }

        if (found)
        {
            cout << BGREEN << "\n  CORRECT! Edge " << u << "-" << v
                 << " severed. Camera cluster goes dark.\n" << RESET;
            gs.gamePassed[3] = true;
            gs.alarmDisabled = true;
            break;
        }
        else
        {
            gs.score[3] -= 5;
            if (gs.score[3] < 0) gs.score[3] = 0;
            if (attempts < 3)
                cout << BRED << "  Wrong edge. Alarms still active. -5 points. Try again.\n" << RESET;
            else
            {
                cout << BRED << "\n  Failed. Alarm stays active. Escape will be harder.\n" << RESET;
                gs.alarmDisabled = false;
                gs.bridgeU = 2;
                gs.bridgeV = 3;
                gs.score[3] = 0;
            }
        }
    }

    cout << "\n  CHAIN: Alarm = "
         << (gs.alarmDisabled ? BGREEN + string("DISABLED") : BRED + string("TRIGGERED"))
         << RESET << "  (affects your escape path in Game 5)\n";
    pressEnter();
}

//   GAME 5 - ESCAPE ROUTE   (Topological Sort + BFS)
void game5()
{
    clearScreen();
    printBanner("GAME 5 - ESCAPE ROUTE");

    cout << "  Evidence downloaded. Time to get out.\n";
    if (gs.alarmDisabled)
    {
        cout << BGREEN << "  Alarm is DISABLED. All paths open.\n" << RESET;
        cout << "  8 rooms. Standard escape route.\n\n";
    }
    else
    {
        cout << BRED << "  Alarm is TRIGGERED. 2 extra doors are locked.\n" << RESET;
        cout << "  8 rooms. Longer path required.\n\n";
    }

    int n = 8;
    clearGraph5();

    if (gs.alarmDisabled)
    {
        depAdj[0].push_back(2); depAdj[0].push_back(3);
        depAdj[1].push_back(3);
        depAdj[2].push_back(4);
        depAdj[3].push_back(5);
        depAdj[4].push_back(6);
        depAdj[5].push_back(6);
        depAdj[6].push_back(7);
        for (int u = 0; u < n; u++)
            for (int j = 0; j < (int)depAdj[u].size(); j++)
                indeg_arr[depAdj[u][j]]++;
        escAdj[0].push_back(1); escAdj[1].push_back(0);
        escAdj[0].push_back(2); escAdj[2].push_back(0);
        escAdj[1].push_back(3); escAdj[3].push_back(1);
        escAdj[2].push_back(4); escAdj[4].push_back(2);
        escAdj[3].push_back(5); escAdj[5].push_back(3);
        escAdj[4].push_back(6); escAdj[6].push_back(4);
        escAdj[5].push_back(6); escAdj[6].push_back(5);
        escAdj[6].push_back(7); escAdj[7].push_back(6);
    }
    else
    {
        depAdj[0].push_back(2);
        depAdj[2].push_back(4);
        depAdj[4].push_back(6);
        depAdj[6].push_back(7);
        for (int u = 0; u < n; u++)
            for (int j = 0; j < (int)depAdj[u].size(); j++)
                indeg_arr[depAdj[u][j]]++;
        escAdj[0].push_back(2); escAdj[2].push_back(0);
        escAdj[2].push_back(4); escAdj[4].push_back(2);
        escAdj[4].push_back(6); escAdj[6].push_back(4);
        escAdj[6].push_back(7); escAdj[7].push_back(6);
    }

    cout << BCYAN << "  ASCII MAP OF THE ESCAPE ROOMS:\n\n" << RESET;
    if (gs.alarmDisabled)
    {
        cout << "  DEPENDENCIES (room X must be cleared before room Y):\n\n";
        cout << "     [0] -------> [2] --> [4] --\\\n";
        cout << "      |                          +--> [6] --> [7] EXIT\n";
        cout << "      +---------> [3] --> [5] --/\n";
        cout << "     [1] -------> [3]\n\n";
        cout << "  MOVEMENT CONNECTIONS (bidirectional doors):\n\n";
        cout << "     [0]--[1]   [0]--[2]   [1]--[3]   [2]--[4]\n";
        cout << "     [3]--[5]   [4]--[6]   [5]--[6]   [6]--[7]\n\n";
        cout << YELLOW << "  To find topo order: start with rooms that have 0 dependencies\n";
        cout << "  (no arrows pointing INTO them). Then remove them and repeat.\n\n" << RESET;
        cout << YELLOW << "  To find BFS hops R0->R7: use movement connections above,\n";
        cout << "  count the minimum number of door-crossings.\n\n" << RESET;
    }
    else
    {
        cout << "  DEPENDENCIES (alarm triggered - fewer paths):\n\n";
        cout << "     [0] --> [2] --> [4] --> [6] --> [7] EXIT\n\n";
        cout << "  MOVEMENT CONNECTIONS (locked doors removed):\n\n";
        cout << "     [0]--[2]   [2]--[4]   [4]--[6]   [6]--[7]\n\n";
        cout << YELLOW << "  Topo order follows the single chain above.\n\n" << RESET;
        cout << YELLOW << "  BFS hops R0->R7: count door-crossings along that chain.\n\n" << RESET;
    }

    cout << BMAGENTA << "  HOW TOPOLOGICAL SORT WORKS:\n" << RESET;
    cout << "  Some rooms only unlock after others are cleared.\n";
    cout << "  Kahn's algorithm: start with rooms that have no dependencies.\n";
    cout << "  Process them, reduce counts, add newly free rooms to queue.\n\n";

    vector<int> topoOrder = topoSort(n);

    cout << "  Room unlock order:\n  ";
    for (int i = 0; i < (int)topoOrder.size(); i++)
    {
        cout << BGREEN << "R" << topoOrder[i] << RESET;
        if (i + 1 < (int)topoOrder.size()) cout << " -> ";
    }
    cout << "\n\n";

    cout << BMAGENTA << "  HOW BFS SHORTEST PATH WORKS:\n" << RESET;
    cout << "  Start from Room 0. Explore all neighbours at distance 1 first.\n";
    cout << "  Then distance 2, then 3, and so on.\n";
    cout << "  First time you reach Room 7 is the shortest path.\n\n";

    int correctHops = bfsHops(n, 0, 7);
    gs.escapeHops   = correctHops;

    cout << "  (Enter H for a hint)\n";
    if (askHint(4))
    {
        cout << BMAGENTA << "\n  HINT - BFS Shortest Path\n" << RESET;
        cout << "  Count the minimum room-to-room moves from R0 to R7.\n";
        if (gs.alarmDisabled)
            cout << "  Connections: 0-1, 0-2, 1-3, 2-4, 3-5, 4-6, 5-6, 6-7\n";
        else
            cout << "  Connections (locked doors removed): 0-2, 2-4, 4-6, 6-7\n";
        cout << "\n";
    }

    gs.score[4] = 20;

    cout << BYELLOW << "\n  PUZZLE 1: Enter the first 4 rooms in the unlock order.\n" << RESET;
    cout << "  (Enter room numbers one by one)\n\n";

    bool topoOk = true;
    for (int i = 0; i < 4; i++)
    {
        int ans = inputInt("  Room " + to_string(i+1) + ": ");
        if (i < (int)topoOrder.size() && ans != topoOrder[i])
            topoOk = false;
    }

    if (topoOk)
        cout << BGREEN << "\n  Correct unlock order! +0 penalty.\n" << RESET;
    else
    {
        cout << BRED << "\n  Wrong order. Some doors resisted. -5 points.\n" << RESET;
        gs.score[4] -= 5;
        if (gs.score[4] < 0) gs.score[4] = 0;
    }

    cout << BYELLOW << "\n  PUZZLE 2: How many hops from Room 0 to Room 7?\n" << RESET;
    cout << "  (minimum room-to-room moves)\n\n";

    int attempts = 0;
    while (attempts < 3)
    {
        int ans = inputInt("  Hops: ");
        attempts++;

        if (ans == correctHops)
        {
            cout << BGREEN << "\n  CORRECT! Shortest path = " << correctHops << " hops.\n" << RESET;
            cout << BGREEN << "  You sprint to the exit. FREEDOM.\n" << RESET;
            gs.gamePassed[4] = true;
            break;
        }
        else
        {
            gs.score[4] -= 5;
            if (gs.score[4] < 0) gs.score[4] = 0;
            if (attempts < 3)
                cout << BRED << "  Wrong. -5 points. Try again.\n" << RESET;
            else
                cout << BRED << "\n  Failed. Correct answer was " << correctHops << " hops.\n" << RESET;
        }
    }

    pressEnter();
}

//   INTRO
void intro()
{
    clearScreen();
    cout << BCYAN;
    cout << "  +=========================================================+\n";
    cout << "  |                  GHOST PROTOCOL                         |\n";
    cout << "  |     One Conspiracy. Five Algorithms. Zero Traces.       |\n";
    cout << "  +=========================================================+\n";
    cout << RESET << "\n";

    cout << "  Year 2031. MegaCorp has buried evidence of a city-wide\n";
    cout << "  conspiracy. A whistleblower sent you one encrypted message\n";
    cout << "  before disappearing. That message is your only lead.\n\n";

    cout << "  You must crack 5 security systems.\n";
    cout << "  Each system's output feeds directly into the next.\n";
    cout << "  One chain. No resets. Every mistake echoes forward.\n\n";

    cout << BRED   << "  Each game gives you 3 attempts before the chain continues.\n" << RESET;
    cout << YELLOW << "  Press H at any puzzle for a hint (-5 points).\n\n" << RESET;

    cout << BMAGENTA << "  ALGORITHMS USED:\n" << RESET;
    cout << "  Game 1 - Sliding Window\n";
    cout << "  Game 2 - XOR + Bit Manipulation\n";
    cout << "  Game 3 - Sprague-Grundy / Nim\n";
    cout << "  Game 4 - Tarjan Bridge Detection + DSU\n";
    cout << "  Game 5 - Topological Sort + BFS\n\n";
}

//   FINAL DEBRIEF
void debrief()
{
    clearScreen();

    int total = 0;
    for (int i = 0; i < 5; i++) total += gs.score[i];

    printBanner("GHOST PROTOCOL - DEBRIEF");

    cout << "  Operative : " << BWHITE << gs.name << RESET << "\n";
    cout << "  Score     : " << BGREEN << total << " / 100\n" << RESET;

    string rank, rankMsg;
    if      (total >= 95) { rank = "PHANTOM";  rankMsg = "You were never here."; }
    else if (total >= 80) { rank = "GHOST";    rankMsg = "Clean, precise, lethal."; }
    else if (total >= 60) { rank = "AGENT";    rankMsg = "Good. But they saw your shadow."; }
    else if (total >= 40) { rank = "BURNED";   rankMsg = "MegaCorp has your face."; }
    else                  { rank = "CAPTURED"; rankMsg = "Game over, ZERO."; }

    cout << "  Rank      : " << BYELLOW << rank << RESET << "\n";
    cout << "  \"" << rankMsg << "\"\n\n";

    printLine();
    cout << BMAGENTA << "  CHAIN RECONSTRUCTION\n" << RESET;
    printLine();
    cout << "  Game 1 Entry Point ID : " << BGREEN << gs.entryPointID  << RESET << "  (Sliding Window)\n";
    cout << "  Game 2 Suspects Found : " << BGREEN << gs.suspectCount  << RESET << "  (XOR Decryption)\n";
    cout << "  Game 3 Nim Piles      : " << BGREEN << gs.nim_pile0 << "," << gs.nim_pile1 << "," << gs.nim_pile2 << RESET << "  (from Suspect Count)\n";
    cout << "  Game 4 Bridge Cut     : " << BGREEN << gs.bridgeU << "-" << gs.bridgeV << RESET << "  (Tarjan Bridge)\n";
    cout << "  Game 5 Escape Hops    : " << BGREEN << gs.escapeHops    << RESET << "  (BFS)\n\n";

    printLine();
    cout << BMAGENTA << "  ALGORITHM REPORT\n" << RESET;
    printLine();

    string gameNames[5] = {
        "Sliding Window   ",
        "XOR Manipulation ",
        "Sprague-Grundy   ",
        "Tarjan + DSU     ",
        "BFS + Topo Sort  "
    };

    for (int i = 0; i < 5; i++)
    {
        cout << "  " << gameNames[i] << " : ";
        if (gs.gamePassed[i]) cout << BGREEN << "PASSED  " << RESET;
        else                   cout << BRED   << "FAILED  " << RESET;
        cout << "  Score: " << gs.score[i] << "/20";
        if (gs.hintsUsed[i] > 0) cout << "  (hints: " << gs.hintsUsed[i] << ")";
        cout << "\n";
    }

    cout << "\n";
    printLine();
    cout << BMAGENTA << "  SCORE BREAKDOWN\n" << RESET;
    printLine();
    for (int i = 0; i < 5; i++)
        cout << "  Game " << i+1 << " : " << gs.score[i] << " / 20\n";
    cout << "  TOTAL  : " << BGREEN << total << " / 100\n" << RESET;

    cout << "\n";
    printLine();
    cout << BMAGENTA << "  CONSPIRACY EXPOSED\n" << RESET;
    printLine();
    cout << "  MegaCorp Director Hayes authorized illegal surveillance\n";
    cout << "  of 40,000 citizens. The evidence is now public.\n";
    cout << "  Hayes is under arrest.\n";
    cout << "  You vanished before anyone knew your name.\n\n";

    if (total >= 80)
    {
        cout << BYELLOW << "  ACHIEVEMENT UNLOCKED\n" << RESET;
        if (total == 100)
            cout << BGREEN << "  PERFECT RUN - Zero hints. Zero mistakes. Zero traces.\n" << RESET;
        else
            cout << BGREEN << "  GHOST PROTOCOL COMPLETE - The chain held.\n" << RESET;
    }

    cout << "\n";
}

//   MAIN
int main()
{


    system("chcp 65001 > nul");
    intro();

    cout << BYELLOW << "  Enter your operative name: " << RESET;
    getline(cin, gs.name);
    if (gs.name.empty()) gs.name = "ZERO";

    initState();

    cout << BGREEN << "\n  Welcome, " << gs.name << ". The chain begins now.\n" << RESET;
    pressEnter();

    game1();
    game2();
    game3();
    game4();
    game5();

    debrief();

    cout << "  Play again? (Y/N): ";
    string again;
    getline(cin, again);
    if (again == "Y" || again == "y")
    {
        initState();
        cout << BYELLOW << "  Enter your operative name: " << RESET;
        getline(cin, gs.name);
        if (gs.name.empty()) gs.name = "ZERO";
        pressEnter();
        game1();
        game2();
        game3();
        game4();
        game5();
        debrief();
    }

    cout << BCYAN << "\n  GHOST PROTOCOL - END TRANSMISSION\n" << RESET;
    return 0;
}
