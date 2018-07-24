#include "stdafx.h"
#include "MPR.h"

void MPR::init()
{
	NW = nUsers + nItems + K * (nUsers + nItems);
	W = new double[NW];
	bestW = new double[NW];

	getParametersFromVector(W, &beta_user, &beta_item, &gamma_user, &gamma_item, INIT);

	for (int u = 0; u < nUsers; u++)
	{
		for (int k = 0; k < K; k++)
		{
			gamma_user[u][k] = rand()*1.0 / RAND_MAX;
		}
	}

	for (int i = 0; i < nItems; i++)
	{
		for (int k = 0; k < K; k++)
		{
			gamma_item[i][k] = rand()*1.0 / RAND_MAX;
		}
	}
}

void MPR::cleanUp()
{
	getParametersFromVector(W, &beta_user, &beta_item, &gamma_user, &gamma_item, FREE);

	delete[] W;
	delete[] bestW;
}

void MPR::getParametersFromVector(double* g, double** beta_user, double** beta_item, double*** gamma_user, double*** gamma_item, action_t action)
{
	if (action == FREE)
	{
		delete[](*gamma_user);
		delete[](*gamma_item);
		return;
	}

	if (action == INIT)
	{
		*gamma_user = new double*[nUsers];
		*gamma_item = new double*[nItems];
	}

	int ind = 0;

	*beta_user = g + ind;
	ind += nUsers;

	*beta_item = g + ind;
	ind += nItems;

	for (int u = 0; u < nUsers; u++)
	{
		(*gamma_user)[u] = g + ind;
		ind += K;
	}
	for (int i = 0; i < nItems; i++)
	{
		(*gamma_item)[i] = g + ind;
		ind += K;
	}

	if (ind != NW)
	{
		printf("Got bad index");
		exit(1);
	}
}

double MPR::prediction(int user, int item)
{
	return beta_user[user] + beta_item[item] + inner(gamma_user[user], gamma_item[item], K);
}

int MPR::sampleUser()
{
	while (true)
	{
		int user_id = rand() % nUsers;
		if (pos_per_user[user_id].size() == 0 || (int)pos_per_user[user_id].size() == nItems)
		{
			continue;
		}
		return user_id;
	}
}

int MPR::sampleItem()
{
	while (true)
	{
		int item_id = rand() % nItems;
		if (pos_per_item[item_id].size() == 0 || (int)pos_per_item[item_id].size() == nUsers)
		{
			continue;
		}
		return item_id;
	}
}

void MPR::updateFactors(int user_id, int pos_item_id, int neg_item_id, int item_id, int pos_user_id, int neg_user_id, double learn_rate, double alpha)
{
	// item ranking part
	double learn_rate_IR = (1.0 - alpha)*learn_rate;
	double x_uij = beta_item[pos_item_id] - beta_item[neg_item_id];
	x_uij += inner(gamma_user[user_id], gamma_item[pos_item_id], K) - inner(gamma_user[user_id], gamma_item[neg_item_id], K);

	double deri = 1.0 / (1 + exp(x_uij));

	beta_item[pos_item_id] += learn_rate_IR * (deri - biasReg * beta_item[pos_item_id]);
	beta_item[neg_item_id] += learn_rate_IR * (-deri - biasReg * beta_item[neg_item_id]);

	// adjust latent factors
	for (int f = 0; f < K; f++)
	{
		double w_uf = gamma_user[user_id][f];
		double h_if = gamma_item[pos_item_id][f];
		double h_jf = gamma_item[neg_item_id][f];

		gamma_user[user_id][f] += learn_rate_IR * (deri * (h_if - h_jf) - lambda * w_uf);
		gamma_item[pos_item_id][f] += learn_rate_IR * (deri * w_uf - lambda * h_if);
		gamma_item[neg_item_id][f] += learn_rate_IR * (-deri * w_uf - lambda * h_jf);
	}
	
	// audience retrieval part
	double learn_rate_AR = alpha * learn_rate;
	double y_ivw = beta_user[pos_user_id] - beta_user[neg_user_id];
	y_ivw += inner(gamma_user[pos_user_id], gamma_item[item_id], K) - inner(gamma_user[neg_user_id], gamma_item[item_id], K);

	deri = 1.0 / (1 + exp(y_ivw));

	beta_user[pos_user_id] += learn_rate_AR * (deri - biasReg * beta_user[pos_user_id]);
	beta_user[neg_user_id] += learn_rate_AR * (-deri - biasReg * beta_user[neg_user_id]);

	// adjust latent factors
	for (int f = 0; f < K; f++)
	{
		double w_if = gamma_item[item_id][f];
		double h_vf = gamma_user[pos_user_id][f];
		double h_wf = gamma_user[neg_user_id][f];

		gamma_item[item_id][f] += learn_rate_AR * (deri * (h_vf - h_wf) - lambda * w_if);
		gamma_user[pos_user_id][f] += learn_rate_AR * (deri * w_if - lambda * h_vf);
		gamma_user[neg_user_id][f] += learn_rate_AR * (-deri * w_if - lambda * h_wf);
	}
}

void MPR::oneiteration(double learn_rate, double alpha)
{
	int user_id, pos_item_id, neg_item_id;
	vector<int>* user_matrix = new vector<int>[nUsers];
	int item_id, pos_user_id, neg_user_id;
	vector<int>* item_matrix = new vector<int>[nItems];
	for (int u = 0; u < nUsers; u++)
	{
		for (map<int, long long>::iterator it = pos_per_user[u].begin(); it != pos_per_user[u].end(); it++)
		{
			user_matrix[u].push_back(it->first);
		}
	}
	for (int i = 0; i < nItems; i++)
	{
		for (map<int, long long>::iterator it = pos_per_item[i].begin(); it != pos_per_item[i].end(); it++)
		{
			item_matrix[i].push_back(it->first);
		}
	}

	for (int i = 0; i < num_pos_events; i++)
	{
		// for item ranking
		// sample a user
		user_id = sampleUser();
		vector<int>& user_items = user_matrix[user_id];

		if (user_items.size() == 0)
		{
			for (map<int, long long>::iterator it = pos_per_user[user_id].begin(); it != pos_per_user[user_id].end(); it++)
			{
				user_items.push_back(it->first);
			}
		}

		// sample pos item
		int rand_num = rand() % user_items.size();
		pos_item_id = user_items.at(rand_num);
		user_items.at(rand_num) = user_items.back();
		user_items.pop_back();

		// for audience retrieval
		// sample neg item
		do
		{
			neg_item_id = rand() % nItems;
		} while (pos_per_user[user_id].find(neg_item_id) != pos_per_user[user_id].end());

		// sample an item
		item_id = sampleItem();
		vector<int>& item_users = item_matrix[item_id];

		if (item_users.size() == 0)
		{
			for (map<int, long long>::iterator it = pos_per_item[item_id].begin(); it != pos_per_item[item_id].end(); it++)
			{
				item_users.push_back(it->first);
			}
		}

		// sample pos user
		rand_num = rand() % item_users.size();
		pos_user_id = item_users.at(rand_num);
		item_users.at(rand_num) = item_users.back();
		item_users.pop_back();

		// sample neg user
		do
		{
			neg_user_id = rand() % nUsers;
		} while (pos_per_item[item_id].find(neg_user_id) != pos_per_item[item_id].end());

		// update factors
		updateFactors(user_id, pos_item_id, neg_item_id, item_id, pos_user_id, neg_user_id, learn_rate, alpha);
	}
	delete[] user_matrix;
	delete[] item_matrix;
}

void MPR::train(int iterations, double learn_rate, double alpha)
{
	fprintf(stderr, "%s", ("\n<<<" + toString() + ">>>\n\n").c_str());

	double bestValidAUC_AR = -1;
	double bestValidAUC_IR = -1;
	int best_iter_AR = 0;
	int best_iter_IR = 0;

	// SGD
	for (int iter = 1; iter <= iterations; iter++)
	{
		double l_dlStart = clock();
		oneiteration(learn_rate, alpha);
		fprintf(stderr, "Iter: %d, took %f\n", iter, clock() - l_dlStart);

		if (iter % 5 == 0)
		{
			double valid_AR, test_AR, std_AR;
			AUC_AR(&valid_AR, &test_AR, &std_AR);
			fprintf(stderr, "[Valid AUC_AR = %f, Test AUC_AR = %f, Test std_AR = %f\n", valid_AR, test_AR, std_AR);

			double valid_IR, test_IR, std_IR;
			AUC_IR(&valid_IR, &test_IR, &std_IR);
			fprintf(stderr, "[Valid AUC_IR = %f, Test AUC_IR = %f, Test std_IR = %f\n", valid_IR, test_IR, std_IR);

			if (bestValidAUC_AR < valid_AR && bestValidAUC_IR < valid_IR)
			{
				bestValidAUC_AR = valid_AR;
				best_iter_AR = iter;
				bestValidAUC_IR = valid_IR;
				best_iter_IR = iter;
				copyBestModel();
			}
			else if ((valid_AR < bestValidAUC_AR && iter > best_iter_AR + 50) || (valid_IR < bestValidAUC_IR && iter > best_iter_IR + 50))
			{
				fprintf(stderr, "Overfitted. Exiting... \n");
				break;
			}
		}
	}
	// copy back bestW
	for (int w = 0; w < NW; w++)
	{
		W[w] = bestW[w];
	}

	double valid_AR, test_AR, std_AR, valid_IR, test_IR, std_IR;
	AUC_AR(&valid_AR, &test_AR, &std_AR);
	AUC_IR(&valid_IR, &test_IR, &std_IR);
	fprintf(stderr, "\n\n <<<MPR>> Test AUC_AR = %f, Test std_AR = %f \n Test AUC_IR = %f, Test std_IR = %f \n", test_AR, std_AR, test_IR, std_IR);
}

string MPR::toString()
{
	char str[10000];
	sprintf_s(str, "MPR_K_%d_lambada_%.2f_biasReg_%.2f_alpha_%.2f", K, lambda, biasReg, alpha);
	return str;
}