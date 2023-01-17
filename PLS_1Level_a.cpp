// This is the code for Product Line Selection.
//Ref: McBride, R. D., Zufryden, F. S. 1988. An integer programming approach to the optimal 
//product line selection problem. Makreting Science, 7 (2), 126-140

#include<stdio.h>
#include<conio.h>
#include<iostream>
#include<fstream>
#include<iosfwd>
#include<string>
#include <deque>
#include <sstream>
#include <time.h>
#include <stdlib.h>

#include <ilcplex/ilocplex.h>
#include <ilconcert/ilosys.h>

ILOSTLBEGIN

typedef IloArray<IloNumArray> TwoDMatrix;
typedef IloArray<TwoDMatrix> ThreeDMatrix;
int main(int argc, char **argv)   
{
	IloEnv env;
	int S;//No. of customer segments
	int P;//No. of products

	IloNumArray Market_size(env);//No. of customers in each Market Segment
	TwoDMatrix Utility(env);//Customer Utility for each product
	IloNumArray FC(env);//Fixed cost of product development
	IloNumArray Unit_profit(env);//profit per unit of a product
	IloNumArray Reserve_util(env);//Reservation utility of each customer segment

	IloNum eps;//later assigned as eps = cplex.getParam(IloCplex::EpInt); EpInt is the tolerance gap for integer variables

	try 
	{
		///////// DATA FILE READING //////////
	
		//const char* data_filename  = "prod_line_select/prod_line_4_5.txt";//4 customer segments; 5 products
		//const char* data_filename = "PLS/Data/PLS_10_5.txt";//4 customer segments; 5 products
		//const char* data_filename = "PLS/Data/PLS_50_5.txt";//5 customer segments; 50 products
		//const char* model_filename = "PLS/Models/PLS_Model_50_5_1a.lp";//5 customer segments; 50 products
		const char* data_filename = "PLS/Data/PLS_200_8.txt";//5 customer segments; 50 products
		const char* model_filename = "PLS/Models/PLS_Model_200_8_1a.lp";//5 customer segments; 50 products
		if (argc > 1)
		{
			data_filename = argv[1];
		}
		fstream datafile;
		datafile.open(data_filename,ios::in);

		if (!datafile) 
		{
			cerr << "ERROR: could not open file " << data_filename << " for reading" << endl;
			cerr << "usage:   " << argv[0] << " <datafile>" << endl;
			throw(-1);
		}

		datafile >> Market_size >> Utility >> FC >>Unit_profit>> Reserve_util;
		cout << Market_size << endl;
		cout << Utility << endl;
		cout << FC << endl;
		cout << Unit_profit << endl;
		cout << Reserve_util << endl;

		S = Market_size.getSize();
		P = FC.getSize();
		cout<<"Number of Customer Segments = "<<S<<endl;
		cout<<"Number of Products = "<<P<<endl;
		
		IloBool consistentData = (Market_size.getSize() == Utility[0].getSize());
		if (!consistentData) 
		{
			cerr << "ERROR: data file '" << data_filename << "' contains inconsistent data" << endl;
			throw(-1);
		}
		datafile.close();

		// READING DONE..................................
		//=================Decleare Variables===============================
		IloModel model(env);
		//typedef IloArray<IloBoolVarArray> array2d;//Creating a 2d array of x variables
		typedef IloArray<IloNumVarArray> array2d;//Creating a 2d array of x variables
		IloBoolVarArray Y(env, P);//Y_p = 1 if product p is developed, 0 otherwise
		array2d X(env, P); //X_pc = 1 if customer segment c buys product p, 0 otherwise
		//=======================================================================
		IloNumArray Y_val(env, P);//to store values of Y variables
		TwoDMatrix X_val(env, P);//to store values of X
		//=======================================================================
		for (int p=0; p<P; p++)
		{
			//X[p]=IloBoolVarArray(env, S);
			X[p] = IloNumVarArray(env, S, 0, 1, ILOFLOAT);
			X_val[p]=IloNumArray(env, S);
		}

		// Objective Function: Minimize: sum{s in 1,..,S} MarketSize_c*sum {p in 1..P}UnitProfit[p]*X[p][s] - sum {p in 1..P} FC[p]*Y[p]
		IloExpr Obj(env); // Creates an expression with the name Obj (Objective)
		for (int p=0;p<P;p++)
		{
			for (int s=0;s<S;s++)
			{
				Obj+=Market_size[s]*Unit_profit[p]*X[p][s];
			}
			Obj-=FC[p]*Y[p];
		}


		// model.add is a function to add constraints and objectives in the CPLEX environment
		model.add(IloMaximize(env,Obj)); // IloMinimize is used for minimization problems
		Obj.end(); // Clear out the memory allocated for the expression 


		//A product is selected by any customer segment only if the product is included in the product line.
		//Constraint 1: for {p in 1..P}, {s in 1,..,S}: X[s][p] <= Y[p];
		for(int p=0;p<P;p++)
		{
			for(int s=0;s<S;s++)
			{
				model.add(X[p][s] <= Y[p]);
			}
		}


		//Each customer segment s buys a maximum of 1 product
		//Constraint 2: for {s in 1,..,S}: sum{p in 1..P} X[p][s] <= 1];
		for(int s=0;s<S;s++)
		{
			IloExpr Tot_prod(env);
			for(int p=0;p<P;p++)
			{
				Tot_prod+=X[p][s];
			}
			model.add(Tot_prod <= 1);
			Tot_prod.end();
		}

		////The selected product should give a non-negative utility to the customer segment
		////Constraint 3: for {p in 1..P}, {s in 1,..,S}: Utility[p][s]*X[p][s] >= 0;
		//for(int c=0;c<C;c++)
		//{
		//	for(int p=0;p<P;p++)
		//	{
		//		model.add(Utility[p][c]*X[p][c] >= 0);
		//	}
		//}

		//The selected product should give Greater than Reservation utility to the customer segment
		//Constraint 3: for {p in 1..P, s in 1,..,S: Utility[p][s]<Reserve_util[s]}: X[p][s] <= 0;
		for (int s = 0;s < S;s++)
		{
			for (int p = 0;p < P;p++)
			{
				if(Utility[p][s] < Reserve_util[s])
				{
					model.add(X[p][s] <= 0);
				}
			}
		}

		//Constraint 4(a): A customer segment buys only the product that gives it the largest utility
		//Constraint 4(a): for {s in 1,..,S}, {p in 1..P}, {q in 1..P}: Utility[p][s] >=  Utility[q][s]*Y[q] - M[s][p][q](1 - X[p][s]);
		//IloNum BigM = 0;//BigM = Maximum of all utilities
		/*for(int p=0;p<P;p++)
		{
			BigM = IloMax(BigM, IloMax(Utility[p]));
		}*/
		ThreeDMatrix BigM(env, S);
		//for (int c = 0;c < C;c++)
		//{
		//	BigM[c] = TwoDMatrix(env, P);
		//	for (int p = 0;p < P;p++)
		//	{
		//		BigM[c][p] = IloNumArray(env, P);
		//	}
		//}
		for (int s = 0;s < S;s++)
		{
			BigM[s] = TwoDMatrix(env, P);
			//cout << "Entered here" << endl;
			for (int p = 0;p < P;p++)
			{
				BigM[s][p] = IloNumArray(env, P);
				for (int q = 0;q < P;q++)
				{
					if (p == q)
					{
						BigM[s][p][q] = 0;
					}
					else
					{
						BigM[s][p][q] = IloMax(Utility[p][s], -1 * Utility[p][s]) + IloMax(Utility[q][s], -1 * Utility[q][s]);
						//IloNum Temp1 = IloMax(Utility[p][s], -1 * Utility[p][s]);
						//IloNum Temp2 = IloMax(Utility[q][s], -1 * Utility[q][s]);
						//BigM[s][p][q] = IloMax(Temp1, Temp2);
					}
				}
			}	
		}
		

		for(int s=0;s<S;s++)
		{
			for(int p=0;p<P;p++)
			{
				for(int q=0;q<P;q++)
				{
					if(p!=q)
					{	//model.add(Utility[j][c]*Y[j] >= Utility[i][c]*Y[i] - BigM*(1 - X[j][c]));
						model.add(Utility[p][s] >= Utility[q][s] * Y[q] - BigM[s][p][q] * (1 - X[p][s]));
					}
				}
			}
		}
		//==============================================================================

		//Optimize
		IloCplex cplex(model);
		cplex.exportModel(model_filename);
		//cplex.exportModel("PLS/Models/PLS_Model_1Level_a.lp");
		//cplex.exportModel("ModelProd_line.lp");
		//cplex.setOut(env.getNullStream()); //This is to supress the output of Branch & Bound Tree on screen
		//cplex.setWarning(env.getNullStream()); //This is to supress warning messages on screen
		eps = cplex.getParam(IloCplex::EpInt);

		cplex.solve();//solving the MODEL

		if (cplex.getStatus() == IloAlgorithm::Infeasible) // if the problem is infeasible
		{
			cout << "Problem is Infeasible" << endl; 
		}
		
		// Print results
		cout << "Max Profit = " << setprecision(8)<< cplex.getObjValue() << endl;

		cout<<"Product \t Developed?: "<<endl;

		for (int p=0;p<P;p++)
		{
			if (cplex.getValue(Y[p]) > 1-eps)
			{
				Y_val[p] = 1;
			}
			else
			{
				Y_val[p] = 0;
			}
		}
		cout<< Y_val<<endl;

		cout<<"p s X[p][s]" << endl;
		for(int s=0;s<S;s++)
		{
			for(int p=0;p<P;p++)
			{
				if (cplex.getValue(X[p][s]) > 1-eps)
				{
					X_val[p][s] = 1;
				}
				else
				{
					X_val[p][s] = 0;
				}
			}
		}
		cout<< X_val<<endl;
		//==================SOLVE LP RELAXATION==================
		IloConversion Y_relax = IloConversion(env, Y, ILOFLOAT);
		model.add(Y_relax);
		for (int p = 0; p < P; p++)
		{
			IloConversion X_relax = IloConversion(env, X[p], ILOFLOAT);
			model.add(X_relax);
		}
		model_filename = "PLS/Models/PLS_Model_200_8_1a_LP.lp";//5 customer segments; 50 products
		//cplex.exportModel("PLS/Models/PLS_Model_50_5_1a_LP.lp");
		cplex.exportModel(model_filename);
		//=================TO GET BACK TO THE ORIGINAL IP====================
		//https://or.stackexchange.com/questions/3703/cplex-12-10-how-can-i-solve-an-lp-relaxation
		//Y_relax.end();
		//Similarly for X_relax in loop
		//===================================================================
		cplex.solve();//solving the MODEL

		cout << "====================================" << endl;
		cout << "LP RELAXATION OBJ = " << setprecision(8) << cplex.getObjValue() << endl;
		cout << "Product \t Developed?: " << endl;
		for (int p = 0;p < P;p++)
		{
			Y_val[p] = cplex.getValue(Y[p]);
		}
		cout << Y_val << endl;

		cout << "p s X[p][s]" << endl;
		for (int s = 0;s < S;s++)
		{
			for (int p = 0;p < P;p++)
			{
				X_val[p][s] = cplex.getValue(X[p][s]);
			}
		}
		cout << "p s X[p][s]" << endl;
		cout << X_val << endl;

		model.end();
		env.end();

		}//end of of try block
		catch (IloException& ex) 
		{
			cerr << "Error: " << ex << endl;
		}
		catch (...) 
		{
			cerr << "Error" << endl;
		}
		return 0;
}