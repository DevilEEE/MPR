#pragma once
#include "model.h"

class MPR : public model
{
public:
	MPR(corpus* corp, int K, double lambda, double biasReg, double alpha)
		: model(corp)
		, K(K)
		, lambda(lambda)
		, biasReg(biasReg) 
		, alpha(alpha) {}

	~MPR() {}

	void init();
	void cleanUp();

	double prediction(int user, int item);
	void getParametersFromVector(double* g, double** beta_user, double** beta_item, double*** gamma_user, double*** gamma_item, action_t action);
	int sampleItem();
	int sampleUser();
	void train(int iterations, double learn_rate, double alpha);
	virtual void oneiteration(double learn_rate, double alpha);
	virtual void updateFactors(int user_id, int pos_item_id, int neg_item_id, int item_id, int pos_user_id, int neg_user_id, double learn_rate, double alpha);
	string toString();

	/* auxiliary variables */
	double * beta_item;
	double * beta_user;
	double ** gamma_user;
	double ** gamma_item;

	/* hyper-parameters */
	int K;
	double lambda;
	double biasReg;
	double alpha;
};