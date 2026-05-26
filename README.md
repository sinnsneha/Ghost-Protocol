<div align="center">



\# 👁️ GHOST PROTOCOL



\### \*One Conspiracy. Five Algorithms. Zero Traces.\*



\[!\[C++](https://img.shields.io/badge/C++-14-00599C?style=for-the-badge\&logo=cplusplus\&logoColor=white)](https://isocpp.org/)

\[!\[Platform](https://img.shields.io/badge/Platform-Windows-0078D6?style=for-the-badge\&logo=windows\&logoColor=white)](https://www.microsoft.com/windows/)

\[!\[License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)](LICENSE)

\[!\[Algorithms](https://img.shields.io/badge/Algorithms-5\_chained-ff006e?style=for-the-badge)]()



\*\*A cyberpunk terminal hacking game where five algorithms chain into one unbroken sequence.\*\*

\*\*Solve them. Or get burned.\*\*



!\[Intro](screenshots/01\_intro.png)



</div>



\---



\## 🌃 The Premise



> Year 2031. MegaCorp has buried evidence of a city-wide conspiracy.

> A whistleblower sent you one encrypted message before disappearing.

> That message is your only lead.

>

> You have one shot. Five systems. No retries.



You're an operative codenamed \*\*ZERO\*\*. Your job: crack five security systems, each one feeding intel into the next. Get one wrong and the chain doesn't break — it \*bends\*, dragging your mistakes forward into every system that follows.



There are no save points. There is no undo. \*\*The chain holds, or it doesn't.\*\*



\---



\## 🧠 What Makes This Different



Most DSA practice happens in a vacuum. You solve a sliding window problem. You move on. You solve a graph problem. You move on. \*\*Each problem forgets the last.\*\*



Ghost Protocol asks a harder question:



> \*What if the answer to one algorithm became the input to a completely different one?\*

Five algorithms. One state machine. Zero room for error.



\---



\## ⚔️ The Five Games



<table>

<tr>

<td width="50%" valign="top">



\### 🔍 Game 1 — Sonar Sweep

\*\*Algorithm:\*\* Sliding Window

\*\*Mission:\*\* Find the longest stretch of inactive guards.



The scanner sweeps the network in pulses. Most slots are watched. A few aren't. Find the longest blind spot and that becomes your entry point.



!\[Game 1](screenshots/02\_game1\_sonar.png)



</td>

<td width="50%" valign="top">



\### 🔐 Game 2 — Vault Breaker

\*\*Algorithm:\*\* XOR + Bit Manipulation

\*\*Mission:\*\* Decrypt the suspect list.



The vault encrypts every byte using your entry point ID as the key. Run it back through XOR and the truth comes out — including how many suspects MegaCorp is hiding.



!\[Game 2](screenshots/03\_game2\_xor.png)



</td>

</tr>

<tr>

<td width="50%" valign="top">



\### 🎲 Game 3 — Shadow Nim

\*\*Algorithm:\*\* Sprague-Grundy Theorem

\*\*Mission:\*\* Beat an unbeatable AI.



An AI guardian blocks the server core. It plays Nim like a god. The piles are seeded by your suspect count. There's only one way to win — and the math knows it before you do.



!\[Game 3](screenshots/04\_game3\_nim.png)



</td>

<td width="50%" valign="top">



\### 🌉 Game 4 — City Blackout

\*\*Algorithm:\*\* Tarjan's Bridge Detection + DSU

\*\*Mission:\*\* Find the edge that breaks the network.



The insider's camera cluster is one cut away from going dark. Find the \*\*bridge edge\*\* — the single connection whose removal severs the graph. Cut wrong, alarms wake up.



!\[Game 4](screenshots/05\_game4\_tarjan.png)



</td>

</tr>

<tr>

<td colspan="2" valign="top">



\### 🏃 Game 5 — Escape Route

\*\*Algorithm:\*\* Topological Sort (Kahn's) + BFS Shortest Path

\*\*Mission:\*\* Get out alive.



Evidence downloaded. Now you run. Rooms unlock in dependency order — clear them wrong and doors lock behind you. Find the unlock order, then find the shortest path to the exit. If you triggered the alarm in Game 4, two doors are gone and the graph collapses into a single chain. Adapt or die.



!\[Game 5](screenshots/06\_game5\_escape.png)



</td>

</tr>

</table>



\---



\## 🎯 Scoring \& Ranks



Each game is worth \*\*20 points\*\*. Hints cost \*\*5 points each\*\*. Failed attempts cost \*\*5 points each\*\*. Max possible: \*\*100\*\*.



| Score | Rank | Verdict |

|:-----:|:-----|:--------|

| 95-100 | 👻 \*\*PHANTOM\*\* | You were never here. |

| 80-94  | 🌫️ \*\*GHOST\*\* | Clean, precise, lethal. |

| 60-79  | 🕴️ \*\*AGENT\*\* | Good. But they saw your shadow. |

| 40-59  | 🔥 \*\*BURNED\*\* | MegaCorp has your face. |

| 0-39   | 💀 \*\*CAPTURED\*\* | Game over, ZERO. |



\---



\## 🛠️ Tech Stack



\- \*\*Language:\*\* C++14

\- \*\*Dependencies:\*\* Standard library only (`<vector>`, `<queue>`, `<string>`)

\- \*\*UI:\*\* Pure ANSI escape codes — no ncurses, no external libraries

\- \*\*Platform:\*\* Windows terminal (uses `windows.h` for codepage setup)



Every algorithm is implemented \*\*from scratch\*\*. No `std::sort`-style shortcuts on the core logic. The point isn't to call a library — the point is to \*be\* the library.



\---



\## 🚀 Build \& Run



```bash

\# Compile

g++ -std=c++14 ghost\_protocol.cpp -o ghost\_protocol.exe



\# Run

./ghost\_protocol.exe

```



Requires:

\- Any C++14-capable compiler (MinGW, MSVC, Clang on Windows)

\- Windows 10+ terminal (or Windows Terminal) for ANSI color support

\- A willingness to fail forward



\---



\## 🧬 The Chain (Implementation)



The five games share a single mutable state object. Each game \*reads\* from it and \*writes\* to it:



```cpp

struct State {

&#x20;   int  entryPointID;    // 🔍 Game 1 writes → 🔐 Game 2 reads (as XOR key)

&#x20;   int  suspectCount;    // 🔐 Game 2 writes → 🎲 Game 3 reads (as pile size)

&#x20;   int  insiderNode;     // 🎲 Game 3 writes → 🌉 Game 4 reads (target to isolate)

&#x20;   bool alarmDisabled;   // 🌉 Game 4 writes → 🏃 Game 5 reads (graph topology)



&#x20;   int  score\[5];

&#x20;   int  hintsUsed\[5];

&#x20;   bool gamePassed\[5];

};

```



This is the entire architecture. One struct. Five algorithms. Every output is somebody else's input.



\---



\## 🎓 What I Learned Building This



\- \*\*Sprague-Grundy theorem\*\* isn't just "XOR the piles" — implementing an unbeatable AI requires finding which pile to reduce \*and to what value\*. The math is one line. Getting it right took an afternoon.

\- \*\*Tarjan's bridge-finding\*\* uses two arrays (`disc` and `low`) that confused me until I drew the recursion tree by hand. The "low" value is the earliest discovery time reachable from a subtree — a deceptively simple definition that took rewriting twice.

\- \*\*Designing chained state\*\* is harder than designing each game alone. You have to decide what each game promises to the next, and how to degrade gracefully when the player fails.

\- \*\*Terminal UI matters more than it should.\*\* A working game with no color looks broken. A broken game with good color looks alive. Spend the 30 minutes on ANSI codes.



\---



\## 🪦 Known Limitations



\- \*\*Windows-only as written.\*\* `windows.h` and `system("chcp 65001")` are used for codepage setup. A cross-platform port is doable but unwritten.

\- \*\*Random seed reuses across replays.\*\* If you play twice in one session, the puzzles repeat. Future version: re-seed properly.

\- \*\*Global state struct.\*\* Readable for a 5-game scope, but a real production codebase would encapsulate per-game state in a class.

\- \*\*No automated tests.\*\* The algorithms work, but they're verified by play, not by unit tests. A bigger version of this project would add `catch2` or similar.



\---



\## 📜 License



MIT — fork it, remix it, break the chain.



\---



<div align="center">



\*\*Ghost Protocol\*\* — built with C++, caffeine, and a healthy disrespect for "just use a library."



\*The chain holds. Or it doesn't.\*



</div>

