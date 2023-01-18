# Product-Line-Selection
Alternate Mathematical Models for Product Line Selection

Reference: Jayaswal, Sachin, and Sinha, Ankur. Bilevel Optimization: Applications, Models and Solution Approaches.

## Files in the package
1. PLS_Bilevel_KKT_SOS1.cpp: CPLEX C++ code for Single-level Reformulation of PLS Using KKT Conditions (and Complementary constraints modeled as SOS1 sets); refet to Section 3.1
2. PLS_Bilevel_KKT_SOS1_Alt.cpp: CPLEX C++ code for Single-level Reformulation of PLS Using KKT Conditions (and Complementary constraints modeled as SOS1 sets) using additional dual variables for non-negativity constraints; refet to Section 3.1
3. PLS_Bilevel_StrongDuality.cpp: CPLEX C++ code for Other Single-level Reformulation of PLS Using Duality-based Approach (and BigM constraints to linearize bilevel terms); refet to Section 3.2
4. PLS_1Level_a.cpp: CPLEX C++ code for Other Single-level Reformulations of PLS using constraint set (56); refer to Section 3.3 and Other(a) in Table 3
5. PLS_1Level_b.cpp: CPLEX C++ code for Other Single-level Reformulations of PLS using constraint set (56-b); refer to Section 3.3 and Other(b) in Table 3
6. PLS_1Level_c.cpp: CPLEX C++ code for Other Single-level Reformulations of PLS using constraint set (56-c); refer to Section 3.3 and Other(c) in Table 3
7. PLS_1Level_d.cpp: CPLEX C++ code for Other Single-level Reformulations of PLS using constraint set (56-d); refer to Section 3.3 and Other(d) in Table 3
8. PLS_1Level_e.cpp: CPLEX C++ code for Other Single-level Reformulations of PLS using constraint set (56-e); refer to Section 3.3 and Other(a) in Table 3

## Installations Required
1. Microsoft Visual Studio 
2. IBM CPLEX Concert Technology (C++)

## Quick instruction for execution
1. Create a Visual C++ Project in Microsfot Visual Studio and link it with CPLEX OR Open and exisiting CPLEX C++ prject (for example, if using CPLEX 22.1.0.0, then open any existing project from \Program Files\IBM\ILOG\CPLEX_Studio221\cplex\examples\x64_windows_msvc14\stat_mdd)
2. Add any one of the .cpp files (from Files in the Package listed above)
3. Build Solution; this creates a .exe file with the same name as the name of the project
