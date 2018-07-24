#pragma once
#include "model.h"

class AR : public model
{
public:
	AR(corpus* corp, int K, double lambda, double biasReg)
		: model(corp)
		, K(K)
		, lambda(lambda)
		, biasReg(biasReg) {}

	~AR() {}

	void init();
	void cleanUp();

	double prediction(int user, int item);
	void getParametersFromVector(double* g, double** beta_user, double*** gamma_user, double*** gamma_item, action_t action);
	int sampleItem();
	void train(int iterations, double learn_rate);
	virtual void oneiteration(double learn_rate);
	virtual void updateFactors(int item_id, int pos_user_id, int neg_user_id, double learn_rate);
	string toString();

	/* auxiliary variables */
	double * beta_user;
	double ** gamma_user;
	double ** gamma_item;

	/* hyper-parameters */
	int K;
	double lambda;
	double biasReg;
};