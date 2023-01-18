# Product-Line-Selection
Alternate Mathematical Models for Product Line Selection

Reference: Jayaswal, Sachin, and Sinha, Ankur. Bilevel Optimization: Applications, Models and Solution Approaches.

## Files in the package
a. ### Codes
1. PLS_Bilevel_KKT_SOS1.cpp: CPLEX C++ code for Single-level Reformulation of PLS Using KKT Conditions (and Complementary constraints modeled as SOS1 sets); refet to Section 3.1
2. PLS_Bilevel_KKT_SOS1_Alt.cpp: CPLEX C++ code for Single-level Reformulation of PLS Using KKT Conditions (and Complementary constraints modeled as SOS1 sets) using additional dual variables for non-negativity constraints; refet to Section 3.1
3. PLS_Bilevel_StrongDuality.cpp: CPLEX C++ code for Other Single-level Reformulation of PLS Using Duality-based Approach (and BigM constraints to linearize bilevel terms); refet to Section 3.2
4. PLS_1Level_a.cpp: CPLEX C++ code for Other Single-level Reformulations of PLS using constraint set (56); refer to Section 3.3 and Other(a) in Table 3
5. PLS_1Level_b.cpp: CPLEX C++ code for Other Single-level Reformulations of PLS using constraint set (56-b); refer to Section 3.3 and Other(b) in Table 3
6. PLS_1Level_c.cpp: CPLEX C++ code for Other Single-level Reformulations of PLS using constraint set (56-c); refer to Section 3.3 and Other(c) in Table 3
7. PLS_1Level_d.cpp: CPLEX C++ code for Other Single-level Reformulations of PLS using constraint set (56-d); refer to Section 3.3 and Other(d) in Table 3
8. PLS_1Level_e.cpp: CPLEX C++ code for Other Single-level Reformulations of PLS using constraint set (56-e); refer to Section 3.3 and Other(a) in Table 3

b. ### Data Files
1. PLS_Data_10_5.txt; refer to Section 3, Table 1
2. PLS_Data_200_8.txt; additional data file not shown in the paper
3. PLS_Data_500_25.txt; refer to Section 3, Table 3

## Installations Required
1. Microsoft Visual Studio 
2. IBM CPLEX Concert Technology (C++)

## Quick instruction for execution
1. Create a Visual C++ Project in Microsfot Visual Studio and link it with CPLEX OR Open and exisiting CPLEX C++ prject (for example, if using CPLEX 22.1.0.0, then open any existing project from \Program Files\IBM\ILOG\CPLEX_Studio221\cplex\examples\x64_windows_msvc14\stat_mdd)
2. Add any one of the .cpp files (from Files in the Package listed above)
3. Build Solution; this creates a .exe file with the same name as the name of the project
4. Copy any of the data files (e.g., PLS_Data_10_5.txt) in the same folder where the .exe file is created
5. Open the command prompt (by searching for "cmd" on windows search bar and clicking on "Command Prompt")
6. Change the path on the command prompt to the location of the .exe file
7. Run the .exe file by typing "xxx.exe <space> <name of the data file> <enter>. For example, if the name of the .exe file is PLS.exe, then type "PLS.exe PLS_Data_10_5.txt <enter> to solve the data instance given in PLS_Data_10_5.txt.
