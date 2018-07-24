#include "stdafx.h"
#include "model.h"

void model::AUC_IR(double* AUC_val, double* AUC_test, double* std)
{
	double* AUC_u_val = new double[nUsers];
	double* AUC_u_test = new double[nUsers];
    #pragma omp parallel for schedule(dynamic)
	for (int u = 0; u < nUsers; u++)
	{
		int item_test = test_per_user[u].first;
		int item_val = val_per_user[u].first;
		// sanity check
		if (item_test == -1 || item_val == -1) 
		{
			AUC_u_val[u] = 0;
			AUC_u_test[u] = 0;
			continue;
		}
		double x_u_test = prediction(u, item_test);
		double x_u_val = prediction(u, item_val);

		int count_val = 0;
		int count_test = 0;
		int max = 0;
		for (int j = 0; j < nItems; j++)
		{
			if (pos_per_user[u].find(j) != pos_per_user[u].end() ||
				item_test == j ||
				item_val == j)
				continue;
			max++;
			double x_uj = prediction(u, j);
			if (x_u_test > x_uj)
				count_test++;
			if (x_u_val > x_uj)
				count_val++;
		}
		if (max == 0)
		{
			AUC_u_val[u] = 0;
			AUC_u_test[u] = 0;
		}
		else
		{
			AUC_u_val[u] = 1.0 * count_val / max;
			AUC_u_test[u] = 1.0 * count_test / max;
		}
	}
	// sum up AUC
	*AUC_val = 0;
	*AUC_test = 0;
	for (int u = 0; u < nUsers; u++)
	{
		*AUC_val += AUC_u_val[u];
		*AUC_test += AUC_u_test[u];
	}
	*AUC_val /= nUsers;
	*AUC_test /= nUsers;

	// calculate std
	double variance = 0;
	for (int u = 0; u < nUsers; u++)
	{
		variance += square(AUC_u_test[u] - *AUC_test);
	}
	*std = sqrt(variance / nUsers);

	delete[] AUC_u_test;
	delete[] AUC_u_val;
}

void model::AUC_AR(double* AUC_val, double* AUC_test, double* std)
{
	double* AUC_i_val = new double[nItems];
	double* AUC_i_test = new double[nItems];
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < nItems; i++)
	{
		int user_test = test_per_item[i].first;
		int user_val = val_per_item[i].first;
		// sanity check
		if (user_test == -1 || user_val == -1)
		{
			AUC_i_val[i] = 0;
			AUC_i_test[i] = 0;
			continue;
		}
		double x_i_test = prediction(user_test, i);
		double x_i_val = prediction(user_val, i);

		int count_val = 0;
		int count_test = 0;
		int max = 0;
		for (int u = 0; u < nUsers; u++)
		{
			if (pos_per_item[i].find(u) != pos_per_item[i].end() ||
				user_test == u ||
				user_val == u)
				continue;
			max++;
			double x_iu = prediction(u, i);
			if (x_i_test > x_iu)
				count_test++;
			if (x_i_val > x_iu)
				count_val++;
		}
		if (max == 0)
		{
			AUC_i_val[i] = 0;
			AUC_i_test[i] = 0;
		}
		else
		{
			AUC_i_val[i] = 1.0 * count_val / max;
			AUC_i_test[i] = 1.0 * count_test / max;
		}
	}
	// sum up AUC
	*AUC_val = 0;
	*AUC_test = 0;
	for (int i = 0; i < nItems; i++)
	{
		*AUC_val += AUC_i_val[i];
		*AUC_test += AUC_i_test[i];
	}
	*AUC_val /= nItems;
	*AUC_test /= nItems;

	// calculate std
	double variance = 0;
	for (int i = 0; i < nItems; i++)
	{
		variance += square(AUC_i_test[i] - *AUC_test);
	}
	*std = sqrt(variance / nItems);

	delete[] AUC_i_test;
	delete[] AUC_i_val;
}

void model::copyBestModel()
{
	for (int w = 0; w < NW; w++)
	{
		bestW[w] = W[w];
	}
}

void model::saveModel(const char* path)
{
	FILE* f = fopen_(path, "w");
	fprintf(f, "{\n");
	fprintf(f, "  \"NW\": %d, \n", NW);

	fprintf(f, "  \"W\":  [");
	for (int w = 0; w < NW; w++)
	{
		fprintf(f, "%f", bestW[w]);
		if (w < NW - 1) fprintf(f, ", ");
	}
	fprintf(f, "]\n");
	fprintf(f, "}\n");
	fclose(f);

	fprintf(stderr, "\nModel saved to %s.\n", path);
}

void model::loadModel(const char* path)
{
	fprintf(stderr, "\n  loading parameters from %s.\n", path);
	ifstream in;
	in.open(path);
	if (!in.good()) 
	{
		fprintf(stderr, "Can't read init solution from %s.\n", path);
		exit(1);
	}
	string line;
	string st;
	char ch;
	while (getline(in, line)) 
	{
		stringstream ss(line);
		ss >> st;
		if (st == "\"NW\":") 
		{
			int nw;
			ss >> nw;
			if (nw != NW) 
			{
				fprintf(stderr, "NW not match.");
				exit(1);
			}
			continue;
		}

		if (st == "\"W\":") {
			ss >> ch; // skip '['
			for (int w = 0; w < NW; w++) 
			{
				if (!(ss >> W[w] >> ch)) 
				{
					fprintf(stderr, "Read W[] error.");
					exit(1);
				}
			}
			break;
		}
	}
	in.close();
}

string model::tostring()
{
	return "Empty Model!";
}