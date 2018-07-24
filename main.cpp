#include "stdafx.h"
#include "corpus.h"
#include "AR.h"
#include "BPR.h"
#include "MPR.h"

void go_AR(corpus* corp, int K, double lambda, double biasReg, int iterations, double learn_rate, const char* corp_name)
{
	AR md(corp, K, lambda, biasReg);
	md.init();
	md.train(iterations, learn_rate);
	md.saveModel((string(corp_name) + "__" + md.toString()).c_str());
	md.cleanUp();
}

void go_BPR(corpus* corp, int K, double lambda, double biasReg, int iterations, double learn_rate, const char* corp_name)
{
	BPR md(corp, K, lambda, biasReg);
	md.init();
	md.train(iterations, learn_rate);
	md.saveModel((string(corp_name) + "__" + md.toString()).c_str());
	md.cleanUp();
}

void go_MPR(corpus* corp, int K, double lambda, double biasReg, int iterations, double learn_rate, double alpha, const char* corp_name)
{
	MPR md(corp, K, lambda, biasReg, alpha);
	md.init();
	md.train(iterations, learn_rate, alpha);
	md.saveModel((string(corp_name) + "__" + md.toString()).c_str());
	md.cleanUp();
}

int main()
{
	const char* reviewPath = "C:/Users/Shinelon/Desktop/VBR2016/ratings_Video_Games.csv";
	int K = 20;
	double biasReg = 0.01;
	double lambda = 10.0;
	double alpha = 0.5;
	int iter = 100;
	const char* corpName = "MPR";
	double learn_rate = 0.01;

	fprintf(stderr, "{ \n");
	fprintf(stderr, "  \"corpus\": \"%s\",\n", reviewPath);

	corpus corp;
	corp.loadData(reviewPath, 10, 10);

	//go_AR(&corp, K, lambda, biasReg, iter, learn_rate, corpName);
	//go_BPR(&corp, K, lambda, biasReg, iter, learn_rate, corpName);
	go_MPR(&corp, K, lambda, biasReg, iter, learn_rate, alpha, corpName);

	corp.cleanUp();
	fprintf(stderr, "}\n");
	system("pause");
	return 0;
}
/*
int main(int argc, char** argv)
{
	srand(0);

	if (argc != 8)
	{
		printf(" Parameters as following: \n");
		printf(" 1. Review file path\n");
		printf(" 2. Latent Feature Dim. (K)\n");
		printf(" 3. biasReg (regularize bias terms)\n");
		printf(" 4. lambda  (regularize general terms)\n");
		printf(" 5. alpha (weight between IR and AR : 0~1) \n");
		printf(" 6. Max #iter \n");
		printf(" 7. corpus name \n");
		exit(1);
	}

	char* reviewPath = argv[1];
	int K = atoi(argv[2]);
	double biasReg = atof(argv[3]);
	double lambda = atof(argv[4]);
	double alpha = atof(argv[5]);
	int iter = atoi(argv[6]);
	char* corpName = argv[7];
	double learn_rate = 0.01;

	fprintf(stderr, "{ \n");
	fprintf(stderr, "  \"corpus\": \"%s\",\n", reviewPath);
	
	corpus corp;
	corp.loadData(reviewPath, 5, 0);

	//go_AR(&corp, K, lambda, biasReg, iter, learn_rate, corpName)
	//go_BPR(&corp, K, lambda, biasReg, iter, learn_rate, corpName)
	go_MPR(&corp, K, lambda, biasReg, iter, learn_rate, alpha, corpName);

	corp.cleanUp();
	fprintf(stderr, "}\n");
	system("pause");
	return 0;
}
*/