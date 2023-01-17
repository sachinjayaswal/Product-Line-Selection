// This is the code for Product Line Selection.
//Ref: McBride, R. D., Zufryden, F. S. 1988. An integer programming approach to the optimal 
//product line selection problem. Makreting Science, 7 (2), 126-140
// This is the code for Product Line Selection.
//Ref: McBride, R. D., Zufryden, F. S. 1988. An integer programming approach to the optimal 
//product line selection problem. Makreting Science, 7 (2), 126-140
//This code uses a Bilevel formulation, which is reduced to 1-level using the KKT conditions of the lower level problem.
//The following KKT conditions are used (Ref: Winston & Venkataramanan, Intro to Math Programming, page 716:
//a. Primal Constraints
//b. Dual Constraints
//c. Complementary Slackness:
//i. Primal Slack * Dual Variable = 0 (also for non-negativity constraints)

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
		const char* data_filename = "PLS/Data/PLS_50_5.txt";//5 customer segments; 50 products
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
		typedef IloArray<array2d> array3d;//Creating a 2d array of x variables
		IloBoolVarArray Y(env, P);//Y_p = 1 if product p is developed, 0 otherwise
		array2d X(env, P); //X_ps = 1 if customer segment c buys product p, 0 otherwise
		array2d Alfa(env, P);//Dual variables
		array2d Gamma(env, P);//Dual variables
		array2d Delta(env, P);//Dual variables for X[p][s] >= 0
		IloNumVarArray Beta(env, S, 0, IloInfinity, ILOFLOAT);//Dual variables
		//=======================================================================
		IloNumArray Y_val(env, P);//to store values of Y variables
		TwoDMatrix X_val(env, P);//to store values of X
		TwoDMatrix Alfa_val(env, P);
		TwoDMatrix Gamma_val(env, P);
		TwoDMatrix Delta_val(env, P);
		IloNumArray Beta_val(env, S);
		//=======================================================================
		for (int p=0; p<P; p++)
		{
			//X[p]=IloBoolVarArray(env, S);
			X[p] = IloNumVarArray(env, S, 0, 1, ILOFLOAT);
			X_val[p]=IloNumArray(env, S);
			Alfa[p] = IloNumVarArray(env, S, 0, IloInfinity, ILOFLOAT);
			Gamma[p] = IloNumVarArray(env, S, 0, IloInfinity, ILOFLOAT);
			Delta[p] = IloNumVarArray(env, S, 0, IloInfinity, ILOFLOAT);//Dual variables for X[p][s] >= 0
			Alfa_val[p] = IloNumArray(env, S);
			Gamma_val[p] = IloNumArray(env, S);
			Delta_val[p] = IloNumArray(env, S);
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

		//Dual Constraints: 
		for (int s = 0;s < S;s++)
		{
			for (int p = 0;p < P;p++)
			{
				if (Utility[p][s] < Reserve_util[s])
				{
					//model.add(Alfa[p][s] + Beta[s] +Gamma[p][s] + Delta[p][s] >= Utility[p][s]);
					model.add(Utility[p][s] - Alfa[p][s] - Beta[s] - Gamma[p][s] + Delta[p][s] == 0);
				}
				else
				{
					//model.add(Alfa[p][s] + Beta[s] >= Utility[p][s]);
					model.add(Utility[p][s] - Alfa[p][s] - Beta[s] + Delta[p][s] == 0);
				}
			}
		}

		////Strong Duality Constraint
		//IloExpr LHS(env);
		//IloExpr RHS(env);
		//for (int s = 0;s < S;s++)
		//{
		//	RHS += Beta[s];
		//	for (int p = 0;p < P;p++)
		//	{
		//		LHS += Utility[p][s] * X[p][s];
		//		RHS += Alfa[p][s] * Y[p];
		//	}
		//}
		//model.add(LHS >= RHS);
		//LHS.end();
		//RHS.end();

		//New Variables for Complimentary Slackness: Primal Slack*Dual Variable = 0
		array2d Primal_slack1(env, P);
		for (int p = 0;p < P;p++)
		{
			//XY[p] = IloNumVarArray(env, S, -IloInfinity, 0, ILOFLOAT);
			Primal_slack1[p] = IloNumVarArray(env, S, -1, 0, ILOFLOAT);
			for (int s = 0;s < S;s++)
			{
				model.add(Primal_slack1[p][s] == X[p][s] - Y[p]);
			}
		}

		//IloNumVarArray X_minus_1(env, S, -IloInfinity, 0, ILOFLOAT);
		IloNumVarArray Primal_slack2(env, S, -1, 0, ILOFLOAT);
		for (int s = 0;s < S;s++)
		{
			IloExpr RHS(env);
			for (int p = 0;p < P;p++)
			{
				RHS+= X[p][s];
			}
			model.add(Primal_slack2[s] == RHS- 1);
			RHS.end();
		}

		//SOS1 Constraints: For Utility[p][s] * X[p][s];
		//array3d SOSvar(env, P);//Dual variables
		IloNumVarArray SOSvar(env, 2);
		for (int s = 0;s < S;s++)
		{
			SOSvar[0] = Primal_slack2[s];
			SOSvar[1] = Beta[s];
			model.add(IloSOS1(env, SOSvar));
			for(int p = 0;p < P;p++)
			{
				SOSvar[0] = Primal_slack1[p][s];
				SOSvar[1] = Alfa[p][s];
				model.add(IloSOS1(env, SOSvar));
				SOSvar[0] = X[p][s];
				SOSvar[1] = Gamma[p][s];
				if (Utility[p][s] < Reserve_util[s])
				{
					model.add(IloSOS1(env, SOSvar));
				}
			}
		}

		//SOS1 Constraints: X[p][s]*Delta[p][s] = 0 (Corresponding to Nen-negativity constraints of the primal)
		for (int s = 0;s < S;s++)
		{
			for (int p = 0;p < P;p++)
			{
				SOSvar[0] = X[p][s];
				SOSvar[1] = Delta[p][s];
				model.add(IloSOS1(env, SOSvar));
			}
		}
		//==============================================================================

		//Optimize
		IloCplex cplex(model);
		cplex.exportModel("PLS/Models/PLS_Model_Bilevel_KKT_SOS1_Alt.lp");
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
			Beta_val[s] = cplex.getValue(Beta[s]);
			for(int p=0;p<P;p++)
			{
				Alfa_val[p][s] = cplex.getValue(Alfa[p][s]);
				if (cplex.isExtracted(Gamma[p][s]))
				{
					Gamma_val[p][s] = cplex.getValue(Gamma[p][s]);
				}
				Delta_val[p][s] = cplex.getValue(Delta[p][s]);
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
		cout << "X Values" << endl;
		cout<< X_val<<endl;
		cout << "Alfa Values" << endl;
		cout << Alfa_val << endl;
		cout << "Beta Values" << endl;
		cout << Beta_val << endl;
		cout << "Gamma Values" << endl;
		cout << Gamma_val << endl;
		cout << "Delta Values" << endl;
		cout << Delta_val << endl;
		//==================SOLVE LP RELAXATION==================
		//IloConversion Y_relax = IloConversion(env, Y, ILOFLOAT);
		//model.add(Y_relax);
		//for (int p = 0; p < P; p++)
		//{
		//	IloConversion X_relax = IloConversion(env, X[p], ILOFLOAT);
		//	model.add(X_relax);
		//}
		//cplex.exportModel("PLS/Models/PLS_Model_LP.lp");
		////=================TO GET BACK TO THE ORIGINAL IP====================
		////https://or.stackexchange.com/questions/3703/cplex-12-10-how-can-i-solve-an-lp-relaxation
		////Y_relax.end();
		////Similarly for X_relax in loop
		////===================================================================
		//cplex.solve();//solving the MODEL

		//cout << "====================================" << endl;
		//cout << "LP RELAXATION OBJ = " << setprecision(8) << cplex.getObjValue() << endl;
		//cout << "Product \t Developed?: " << endl;
		//for (int p = 0;p < P;p++)
		//{
		//	Y_val[p] = cplex.getValue(Y[p]);
		//}
		//cout << Y_val << endl;

		//cout << "p s X[p][s]" << endl;
		//for (int s = 0;s < S;s++)
		//{
		//	for (int p = 0;p < P;p++)
		//	{
		//		X_val[p][s] = cplex.getValue(X[p][s]);
		//	}
		//}
		//cout << "p s X[p][s]" << endl;
		//cout << X_val << endl;

		//model.end();
		//env.end();

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