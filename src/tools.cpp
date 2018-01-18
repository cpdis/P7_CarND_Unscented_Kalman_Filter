#include <iostream>
#include "tools.h"

using Eigen::VectorXd;
using Eigen::MatrixXd;
using std::vector;

Tools::Tools() {}

Tools::~Tools() {}

VectorXd Tools::CalculateRMSE(const vector<VectorXd> &estimations,
                              const vector<VectorXd> &ground_truth) {
  /**
  TODO:
    * Calculate the RMSE here.
    * RMSE code reused from Extended Kalman Filter project
  */
  VectorXd rmse = VectorXd(4);
  rmse << 0, 0, 0, 0;

    if (estimations.size() != ground_truth.size() || estimations.size() == 0){
      std::cout << "Invalid estimation or ground_truth data!" << std::endl;
      return rmse;
    }

    for(unsigned int i=0; i < estimations.size(); ++i){
      VectorXd residuals = estimations[i] - ground_truth[i];
      residuals = residuals.array()*residuals.array();
      rmse += residuals;
    }

    /* Calculate the mean. */
    rmse = rmse / estimations.size();
  
    /* Calculate the square root. */
    rmse = rmse.array().sqrt();
    cout << "RMSE: " << rmse << endl;
    
    // Return the RMSE.
    return rmse;
}