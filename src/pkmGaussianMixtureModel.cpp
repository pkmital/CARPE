// Written by Parag K Mital
// Nov. 2008
/*
 CARPE, The Software" © Parag K Mital, parag@pkmital.com
 
 The Software is and remains the property of Parag K Mital
 ("pkmital") The Licensee will ensure that the Copyright Notice set
 out above appears prominently wherever the Software is used.
 
 The Software is distributed under this Licence: 
 
 - on a non-exclusive basis, 
 
 - solely for non-commercial use in the hope that it will be useful, 
 
 - "AS-IS" and in order for the benefit of its educational and research
 purposes, pkmital makes clear that no condition is made or to be
 implied, nor is any representation or warranty given or to be
 implied, as to (i) the quality, accuracy or reliability of the
 Software; (ii) the suitability of the Software for any particular
 use or for use under any specific conditions; and (iii) whether use
 of the Software will infringe third-party rights.
 
 pkmital disclaims: 
 
 - all responsibility for the use which is made of the Software; and
 
 - any liability for the outcomes arising from using the Software.
 
 The Licensee may make public, results or data obtained from, dependent
 on or arising out of the use of the Software provided that any such
 publication includes a prominent statement identifying the Software as
 the source of the results or the data, including the Copyright Notice
 and stating that the Software has been made available for use by the
 Licensee under licence from pkmital and the Licensee provides a copy of
 any such publication to pkmital.
 
 The Licensee agrees to indemnify pkmital and hold them
 harmless from and against any and all claims, damages and liabilities
 asserted by third parties (including claims for negligence) which
 arise directly or indirectly from the use of the Software or any
 derivative of it or the sale of any products based on the
 Software. The Licensee undertakes to make no liability claim against
 any employee, student, agent or appointee of pkmital, in connection 
 with this Licence or the Software.
 
 
 No part of the Software may be reproduced, modified, transmitted or
 transferred in any form or by any means, electronic or mechanical,
 without the express permission of pkmital. pkmital's permission is not
 required if the said reproduction, modification, transmission or
 transference is done without financial return, the conditions of this
 Licence are imposed upon the receiver of the product, and all original
 and amended source code is included in any transmitted product. You
 may be held legally responsible for any copyright infringement that is
 caused or encouraged by your failure to abide by these terms and
 conditions.
 
 You are not permitted under this Licence to use this Software
 commercially. Use for which any financial return is received shall be
 defined as commercial use, and includes (1) integration of all or part
 of the source code or the Software into a product for sale or license
 by or on behalf of Licensee to third parties or (2) use of the
 Software or any derivative of it for research with the final aim of
 developing software products for sale or license to a third party or
 (3) use of the Software or any derivative of it for research with the
 final aim of developing non-software products for sale or license to a
 third party, or (4) use of the Software to provide any service to an
 external organisation for which payment is received. If you are
 interested in using the Software commercially, please contact pkmital to
 negotiate a licence. Contact details are: parag@pkmital.com
 
 
 *
 *
 */

#include "pkmGaussianMixtureModel.h"
#include <opencv2/opencv.hpp>
#include "ofConstants.h"
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

pkmGaussianMixtureModel::pkmGaussianMixtureModel(double *inputData, int observations, int variables, int map_scalar, int cov_type)
:	m_nObservations(observations), m_nVariables(variables), m_nScale(map_scalar)
{
	
	// Setup the CvMats for the inputdata, covariance, means, and labels
	m_pCvData = cvCreateMat(observations, variables, CV_32FC1);
	m_pCvLabels = cvCreateMat( observations, 1, CV_32SC1 );
	
	// Set the number of parameters for the free covariance matrix
	m_nPars = (variables + variables * (variables + 1) / 2); 
	m_nParsOver2 = m_nPars / 2;
	
	
	// For n observations in d dimensions, inputData must have n rows and d columns
	for( int n = 0; n < observations; n++ )
	{
		for( int d = 0; d < variables; d++ )
		{
			((float*)(m_pCvData->data.ptr + m_pCvData->step*n))[d] = inputData[n*variables+d]/(float)map_scalar; 
			//printf("(%d,%d): %f\n", n,d,((float*)(m_pCvData->data.ptr + m_pCvData->step*n))[d]);
		}
	}
	
	if(cov_type == COV_SPHERICAL)
		m_covType = CvEM::COV_MAT_SPHERICAL;//CvEM::COV_MAT_DIAGONAL;////CvEM::COV_MAT_GENERIC;
	else if(cov_type == COV_DIAGONAL)
		m_covType = CvEM::COV_MAT_DIAGONAL;
	else
		m_covType = CvEM::COV_MAT_GENERIC;
    
    bModeled = false;
	
}

pkmGaussianMixtureModel::~pkmGaussianMixtureModel()
{
	cvReleaseMat(&m_pCvData);
	//	cvReleaseMat(&m_pCvMus);
	//	cvReleaseMat(&m_pCvCovs);
	cvReleaseMat(&m_pCvLabels);
	//delete [] emModel;
	//	delete [] m_pData;
}


void pkmGaussianMixtureModel::modelData(int minComponents, int maxComponents, 
										double regularizingFactor, double stoppingThreshold)
{
	
	// indicator will contain the assignments of each data point to
	// the mixture components, as result of the E-step
	//	double * indicator = new double[k * m_nObservations];
	////////////////////////////////////////////////////////////
	
	
	
	////////////////////////////////////////////////////////////
	//
	//	Use as an initial approxiamation, a diagonal covariance matrix
	//	taken from the mean covariances
	//	Could instead use K-Means (see opencv function, kmeans2)
	//
	//	Alternatively, the algorithm may start with M-step when 
	//	initial values for pi,k can be provided. Another alternative, 
	//	when pi,k are unknown, is to use a simpler clustering algorithm 
	//	to pre-cluster the input samples and thus obtain initial pi,k. 
	//	Often (and in ML) k-means algorithm is used for that purpose.
	//
	//	One of the main that EM algorithm should deal with is the large 
	//	number of parameters to estimate. The majority of the parameters 
	//	sits in covariation matrices, which are d×d elements each 
	//	(where d is the feature space dimensionality). However, in many 
	//	practical problems the covariation matrices are close to diagonal, 
	//	or even to μk*I, where I is identity matrix and μk is 
	//	mixture-dependent "scale" parameter. So a robust computation 
	//	scheme could be to start with the harder constraints on the 
	//	covariation matrices and then use the estimated parameters as an 
	//	input for a less constrained optimization problem (often a 
	//	diagonal covariation matrix is already a good enough approximation).
	//
	//	References:
	//
	//	1. [Bilmes98] J. A. Bilmes. A Gentle Tutorial of the EM Algorithm 
	//	and its Application to Parameter Estimation for Gaussian Mixture 
	//	and Hidden Markov Models. Technical Report TR-97-021, 
	//	International Computer Science Institute and Computer Science 
	//	Division, University of California at Berkeley, April 1998.
	
	//// This code is for indexing (observations x variables) 
	
	emModel = new CvEM[maxComponents-minComponents+1];
	
	////////////////////////////////////////////////////////////
	// EM
	int i;
	double minBIC = HUGE_VAL;
	if(maxComponents >= m_nObservations)
	{
		maxComponents = m_nObservations-1;
	}
	if(minComponents > maxComponents)
	{
		minComponents = maxComponents = m_nObservations-1;
	}
	for (int k = minComponents; k <= maxComponents; k++)
	{
#if 0
		//////////////////////////////////////////////////////////////
		// Create a list of random indexes from 1 : K 
		// from the permutations of the number of observations
		int * randIndex = new int[m_nObservations];
		
		// 1:N
		for (i = 0; i < m_nObservations; i++)
			randIndex[i] = i;
		// Shuffle the array
		for (i = 0; i < (m_nObservations-1); i++) 
		{
			// Random position
			int r = i + (rand() % (m_nObservations-i)); 
			// Swap
			int temp = randIndex[i]; randIndex[i] = randIndex[r]; randIndex[r] = temp;
		}
		//////////////////////////////////////////////////////////////
		
		////////////////////////////////////////////////////////////
		// Random initial kernels
		float * estMU = new float[k*m_nVariables];
		for( int row = 0; row < k; row++ )
		{	
			int ind = randIndex[row];
			for( int col = 0; col < m_nVariables; col++ )
			{
				// Get each variable at index ind (of the random kernels)
				// from the input data into estMu
				estMU[row*m_nVariables+col] = ((float*)(m_pCvData->data.ptr + m_pCvData->step*ind))[col];
			}
		}
		CvMat param_mean;
		cvInitMatHeader(&param_mean, k, m_nVariables, CV_32FC1, estMU);
		////////////////////////////////////////////////////////////
		
		////////////////////////////////////////////////////////////
		// Calculate the Covariance matrix (assume this is a 2x2 Matrix)
		CvMat *m_pCvCov = cvCreateMat(m_nVariables, m_nVariables, CV_32FC1);
		CvMat *m_pCvMu = cvCreateMat(m_nVariables, 1, CV_32FC1);
		CvMat **dat = (CvMat**)cvAlloc( m_nObservations * sizeof(*dat) );
		for (i = 0; i < m_nObservations; i++)
		{
			CvMat *tempData = cvCreateMat(m_nVariables, 1, CV_32FC1);
			CV_MAT_ELEM(*tempData, float, 0, 0) = CV_MAT_ELEM(*m_pCvData, float, i, 0);
			CV_MAT_ELEM(*tempData, float, 1, 0) = CV_MAT_ELEM(*m_pCvData, float, i, 1);
			dat[i] = tempData;
		}
		cvCalcCovarMatrix((const CvArr**)dat, m_nObservations, m_pCvCov, 
						  m_pCvMu, CV_COVAR_NORMAL);	//|CV_COVAR_SCALE);
		
		// Store k (all axes) Matrices of Diagonal Covariance Matrices 
		// initialized to 1/10th of the max of the diag values 
		// of the mean variance as the estimated covariances
		CvMat **param_cov = (CvMat**)cvAlloc( k * sizeof(*param_cov) );
		float covMax = MAX(CV_MAT_ELEM(*m_pCvCov, float, 0, 0), CV_MAT_ELEM(*m_pCvCov, float, 1, 1)) / 10.;
		for (int kern = 0; kern < k; kern++)
		{
			CvMat *tempData = cvCreateMat(m_nVariables, m_nVariables, CV_32FC1);
			CV_MAT_ELEM(*tempData, float, 0, 0) = covMax;
			CV_MAT_ELEM(*tempData, float, 0, 1) = 0.0f;
			CV_MAT_ELEM(*tempData, float, 1, 0) = 0.0f;
			CV_MAT_ELEM(*tempData, float, 1, 1) = covMax;
			param_cov[kern] = tempData;
		}
		////////////////////////////////////////////////////////////
		
		////////////////////////////////////////////////////////////
		// Random mixing probabilities for each kernel
		float * estPP = new float[k];
		for (i = 0; i < k; i++)
		{
			estPP[i] = 1.0/(float)k;
		}
		// Weights for each kernel
		CvMat param_weight;
		cvInitMatHeader(&param_weight, k, 1, CV_32FC1, estPP);
		////////////////////////////////////////////////////////////
		
		////////////////////////////////////////////////////////////
		float *estProb = new float[k*m_nObservations];
		for (i = 0; i < k; i++)
		{
			for(int j = 0; j < m_nObservations; j++)
			{
				estProb[i*j] = estPP[i] / 2.0;
			}
		}
		// Create a Cv Matrix for the mix prob
		CvMat param_prob;
		cvInitMatHeader(&param_prob, m_nObservations, k, CV_32FC1, estProb);
		////////////////////////////////////////////////////////////
		
		
		
		// Initialize parameters
		CvEMParams emParam;
		emParam.covs = (const CvMat **)param_cov;
		emParam.means = &param_mean;
		emParam.weights = &param_weight;
		emParam.probs = NULL;//&param_prob;
		emParam.nclusters = k+1;
		emParam.cov_mat_type = CvEM::COV_MAT_GENERIC;//CvEM::COV_MAT_DIAGONAL;////CvEM::COV_MAT_SPHERICAL;
		emParam.start_step = CvEM::START_E_STEP; //CvEM::START_AUTO_STEP;		// initialize with k-means
		emParam.term_crit.epsilon = 0.00001;
		emParam.term_crit.max_iter = 50;
		emParam.term_crit.type = CV_TERMCRIT_ITER | CV_TERMCRIT_EPS;
		
		// Train
		emModel[k-minComponents].train(m_pCvData, 0, emParam, 0);
		
		double thisLikelihood = emModel[k-minComponents].get_log_likelihood();
		//double BIC = -2.*thisLikelihood - (double)k*log((double)m_nObservations*10);
		double BIC = -m_nObservations*thisLikelihood + k/2.*log((double)m_nObservations);
		printf("K: %d, BIC: %f\n", k, BIC);
		if (BIC < minLikelihood)
		{
			bestModel = k-minComponents;
			minLikelihood = BIC;
		}
		
		delete [] randIndex;
		delete [] estMU;
		delete [] estPP;
#else
		CvEMParams emParam;
		emParam.covs = NULL;
		emParam.means = NULL;
		emParam.weights = NULL;
		emParam.probs = NULL;
		emParam.nclusters = k;
		emParam.cov_mat_type = m_covType;//CvEM::COV_MAT_SPHERICAL;//CvEM::COV_MAT_DIAGONAL;////CvEM::COV_MAT_GENERIC;//;
		emParam.start_step = CvEM::START_AUTO_STEP; //CvEM::START_AUTO_STEP;		// initialize with k-means
		emParam.term_crit.epsilon = 0.01;
		emParam.term_crit.max_iter = 100;
		emParam.term_crit.type = CV_TERMCRIT_ITER | CV_TERMCRIT_EPS;
		
		
		// Train
		emModel[k-minComponents].train(m_pCvData, 0, emParam, 0);
		
		// Calculate the log likelihood of the model
		const CvMat *weights = emModel[k-minComponents].get_weights();
		const CvMat *probs = emModel[k-minComponents].get_probs();
		const CvMat **modelCovs = emModel[k-minComponents].get_covs();
		const CvMat *modelMus = emModel[k-minComponents].get_means();
		const CvMat *modelWeights = emModel[k-minComponents].get_weights();
		
		double thisLikelihood;
		if(k == 1)
			// mlem.cpp does not calculate the log_likelihood for 1 cluster 
			// (why i have no idea?! it sets log_likelihood = DBL_MAX/1000.;!?)
			// so i compute it here.  though this seems to pair up with the 
			// same value you get for 2 kernels, it does not pair up for 
			// anything higher?
		{
			double _log_likelihood = 0;//-CV_LOG2PI * (double)m_nObservations * (double)m_nVariables / 2.;
			CvMat *pts = cvCreateMat(m_nVariables, 1, CV_64FC1);
			CvMat *mean = cvCreateMat(m_nVariables, 1, CV_64FC1);
			
			for( int n = 0; n < m_nObservations; n++ )
			{
				double sum = 0;
				cvmSet(pts, 0, 0, cvmGet(m_pCvData, n, 0));
				cvmSet(pts, 1, 0, cvmGet(m_pCvData, n, 1));
				double* pp = (double*)(probs->data.ptr + probs->step*n);
				
				for( int d = 0; d < k; d++ )
				{
					const CvMat * covar = modelCovs[d];
					
					cvmSet(mean, 0, 0, cvmGet(modelMus, d, 0));
					cvmSet(mean, 1, 0, cvmGet(modelMus, d, 1));
					
					double p_x = multinormalDistribution(pts, mean, covar);
					double w_k = cvmGet(weights, 0, d);
					sum += p_x * w_k;// * pp[d];
					//printf("%f + %f += %f\n", p_x, w_k, sum);
				}
				
				_log_likelihood -= log(sum);
			}
			thisLikelihood = -_log_likelihood;//emModel[k-minComponents].get_log_likelihood();
		}
		else
		{
			thisLikelihood = emModel[k-minComponents].get_log_likelihood();
		}
		
		// Calculate the Bit Information Criterion for Model Selection
		double vars = (double)m_nVariables; 
		double N_p = ((double)k-1.)+(double)k*(vars + vars*(vars+1.)/2.);
		double BIC = -2.*thisLikelihood + N_p*log((double)m_nObservations);
		//printf("K: %d, like: %f, BIC: %f\n", k, thisLikelihood, BIC);
		if (BIC < minBIC)
		{
			// update variables with the best bic and best model subscript
			bestModel = k-minComponents;
			minBIC = BIC;
			
			// store the bic and likelihood for printing later
			m_BIC = BIC;
			m_Likelihood = thisLikelihood;
		}
		
#endif
		
	}
    bModeled = true;
	//	m_pCvProb = emModel.get_probs;
	
	
}
double pkmGaussianMixtureModel::multinormalDistribution(const CvMat *pts, const CvMat *mean, const CvMat *covar)
{
	
	int dimensions = 2;
	//  add a tiny bit because of small samples
	CvMat *covarShifted = cvCreateMat(2, 2, CV_64FC1);
	cvAddS( covar, cvScalarAll(0.001), covarShifted);
	
	// calculate the determinant
	double det = cvDet(covarShifted);
	
	// invert covariance
	CvMat *covarInverted = cvCreateMat(2, 2, CV_64FC1);
	cvInvert(covarShifted, covarInverted);
	
	double ff = (1.0/(2.0*(double)PI))*(pow(det,-0.5));
	
	CvMat *centered = cvCreateMat(2, 1, CV_64FC1);
	cvSub(pts, mean, centered);
	
	CvMat *invxmean = cvCreateMat(2, 1, CV_64FC1);
	//cvGEMM(covarInverted, centered, 1., NULL, 1., invxmean);
	cvMatMul(covarInverted, centered, invxmean);
	
	cvMul(centered, invxmean, invxmean);
	CvScalar sum = cvSum(invxmean);
	/*
	 printf("covar: %f %f %f %f\n", cvmGet(covar, 0, 0), cvmGet(covar, 0, 1), cvmGet(covar, 1, 0), cvmGet(covar, 1, 1));
	 printf("covarShifted: %f %f %f %f\n", cvmGet(covarShifted, 0, 0), cvmGet(covarShifted, 0, 1), cvmGet(covarShifted, 1, 0), cvmGet(covarShifted, 1, 1));
	 printf("det: %f\n", det);
	 printf("covarInverted: %f %f %f %f\n", cvmGet(covarInverted, 0, 0), cvmGet(covarInverted, 0, 1), cvmGet(covarInverted, 1, 0), cvmGet(covarShifted, 1, 1));
	 printf("ff: %f\n", ff);
	 printf("pts: %f %f)\n", cvmGet(pts, 0, 0), cvmGet(pts, 1, 0));
	 printf("mean: %f %f)\n", cvmGet(mean, 0, 0), cvmGet(mean, 1, 0));
	 printf("centered: %f %f)\n", cvmGet(centered, 0, 0), cvmGet(centered, 1, 0));
	 printf("invxmean: %f %f)\n", cvmGet(invxmean, 0, 0), cvmGet(invxmean, 1, 0));
	 printf("scalar: %f %f %f %f\n", sum.val[0], sum.val[1], sum.val[2], sum.val[3]);
	 */
	cvReleaseMat(&covarShifted);
	cvReleaseMat(&covarInverted);
	cvReleaseMat(&centered);
	cvReleaseMat(&invxmean);
	
	return ff * exp(-0.5*sum.val[0]);
	
}

/*void pkmGaussianMixtureModel::getLikelihood(int x, int y)
 {
 
 }
 */
void pkmGaussianMixtureModel::getLikelihoodMap(int rows, int cols, unsigned char *map, ofstream &filePtr, int widthstep)
{
	if(widthstep == 0)
        widthstep = cols;
 
    if(!bModeled)
        return;
    
	CvEM myModel = emModel[bestModel];
	const CvMat **modelCovs = myModel.get_covs();
	const CvMat *modelMus = myModel.get_means();
	const CvMat *modelWeights = myModel.get_weights();
	int numClusters = myModel.get_nclusters();
	CvMat *pts = cvCreateMat(m_nVariables, 1, CV_64FC1);
	CvMat *mean = cvCreateMat(m_nVariables, 1, CV_64FC1);
	
	double p = 0;
	double weight;
	double max = 0;
	double prob;
	filePtr << "clusters: " << numClusters << "\n";
	filePtr << "likelihood: " << m_Likelihood << "\n";
	filePtr << "BIC: " << m_BIC << "\n";
	//printf("clusters: %d\n", numClusters);
	
	for (int k = 0; k < numClusters; k++)
	{
		const CvMat * covar = modelCovs[k];
		//printf("covar: (%f, %f, %f, %f)\n", covar->data.ptr[0], \
		covar->data.ptr[1], \
		covar->data.ptr[2], \
		covar->data.ptr[3]);
		//const double *meanVals = (const double *)(modelMus->data.ptr + k*modelWeights->step);
		
		cvmSet(mean, 0, 0, cvmGet(modelMus, k, 0));
		cvmSet(mean, 1, 0, cvmGet(modelMus, k, 1));
		
		//const double *weightPtr = (const double *)(modelWeights->data.ptr + k*modelWeights->step);
		//weight = weightPtr[k];
		weight = cvmGet(modelWeights, 0, k);
		
		filePtr << "mean: " << cvmGet(modelMus, k, 0)*(double)m_nScale << " " << cvmGet(modelMus, k, 1)*(double)m_nScale << "\n";
		
		filePtr << "covar: " << cvmGet(covar, 0, 0) << "\n";
		
		filePtr << "weight: " << weight << "\n";
		
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				cvmSet(pts, 0, 0, (double)j);
				cvmSet(pts, 1, 0, (double)i);	
				prob = multinormalDistribution(pts, mean, covar);
				map[j+i*widthstep] += (int)((weight * prob)*(double)(rows*cols));				
				
			}
		}
	}
	cvReleaseMat(&mean);
	cvReleaseMat(&pts);
}

int pkmGaussianMixtureModel::getNumberOfClusters()
{
	return emModel[bestModel].get_nclusters();	
}

float* pkmGaussianMixtureModel::getClusterMean(int clusterNum)
{
	float *returnedMeans = new float[m_nVariables];
	
	CvEM myModel = emModel[bestModel];
	
	const CvMat *modelMus = myModel.get_means();
	
	int numClusters = myModel.get_nclusters();
	
	for (int i = 0; i < m_nVariables; i++)
	{
		returnedMeans[i] = cvmGet(modelMus, clusterNum, i);
	}
	return returnedMeans;
}

float pkmGaussianMixtureModel::getClusterWeight(int clusterNum)
{
	const CvMat *modelWeights = emModel[bestModel].get_weights();
	
	const double *weightPtr = (const double *)(modelWeights->data.ptr + clusterNum*modelWeights->step);
	
	return weightPtr[clusterNum];
}


float** pkmGaussianMixtureModel::getClusterCov(int clusterNum)
{
	float **returnedCov = new float*[m_nVariables];
	for ( int i = 0; i < m_nVariables; i++ )
		returnedCov[i] = new float[m_nVariables];
	
	CvEM myModel = emModel[bestModel];
	
	const CvMat **modelCovs = myModel.get_covs();
	
	const CvMat * covar = modelCovs[clusterNum];
	
	for (int i = 0; i < m_nVariables; i++)
	{
		for (int j = 0; j < m_nVariables; j++)
		{
			returnedCov[i][j] = cvmGet(covar, i, j);
		}
	}
	return returnedCov;
}


int pkmGaussianMixtureModel::writeToFile(ofstream &fileStream, bool writeClusterNums, bool writeWeights, bool writeMeans, bool writeCovs, bool verbose)
{
	if(!fileStream.is_open())
		return -1;
	
	// use the best-model 
	CvEM myModel = emModel[bestModel];
	const CvMat **modelCovs = myModel.get_covs();
	const CvMat *modelMus = myModel.get_means();
	const CvMat *modelWeights = myModel.get_weights();
	int numClusters = myModel.get_nclusters();
	
	// output the total number of clusters
	if(writeClusterNums)
	{
		if(verbose)
			fileStream << "Number of Clusters: " << numClusters << "\n";
		else
			fileStream << numClusters << "\n";
	}
	
	if(writeWeights)
	{
		// output the weights of each cluster
		if(verbose)
			fileStream << "Weight of Clusters\n";
		for (int k = 0; k < numClusters; k++)
		{
			const double *weightPtr = (const double *)(modelWeights->data.ptr + k*modelWeights->step);
			double weight = weightPtr[0];
			if(verbose)
				fileStream << k << ": " << weight << "\n";
			else
				fileStream << weight << " ";
		}
		fileStream << "\n";
	}
	
	if(writeMeans)
	{
		// output the means of each cluster
		if(verbose)
			fileStream << "Means of Clusters\n";
		for (int k = 0; k < numClusters; k++)
		{
			if(verbose)
				fileStream << "Cluster " << k << ":\n";
			for (int i = 0; i < m_nVariables; i++)
			{
				if(verbose)
					fileStream << i << ": " << cvmGet(modelMus, k, i) << " ";
				else
					fileStream << cvmGet(modelMus, k, i) << " ";
			}
			fileStream << "\n";
		}
	}
	
	if(writeCovs)
	{
		// output the covariances of each cluster
		if(verbose)
			fileStream << "Covariances of Clusters\n";
		for (int k = 0; k < numClusters; k++)
		{
			const CvMat * covar = modelCovs[k];
			if(verbose)
				fileStream << "Cluster " << k << ":\n";
			for (int i = 0; i < m_nVariables; i++)
			{
				for (int j = 0; j < m_nVariables; j++)
				{
					if(verbose)
						fileStream << i << "," << j << ": " << cvmGet(covar, i, j) << " ";
					else
						fileStream << cvmGet(covar, i, j) << " ";
				}
			}
			fileStream << "\n";
		}
	}
	return 0;
}