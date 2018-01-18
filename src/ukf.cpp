#include "ukf.h"
#include "tools.h"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/**
 * Initializes Unscented Kalman filter
 */
UKF::UKF() {
  // if this is false, laser measurements will be ignored (except during init)
  use_laser_ = true;

  // if this is false, radar measurements will be ignored (except during init)
  use_radar_ = true;

  // initial state vector
  x_ = VectorXd(5);

  // initial covariance matrix
  P_ = MatrixXd(5, 5);

  // Process noise standard deviation longitudinal acceleration in m/s^2
  std_a_ = 2.0;

  // Process noise standard deviation yaw acceleration in rad/s^2
  std_yawdd_ = 0.25;

  // Laser measurement noise standard deviation position1 in m
  std_laspx_ = 0.15;

  // Laser measurement noise standard deviation position2 in m
  std_laspy_ = 0.15;

  // Radar measurement noise standard deviation radius in m
  std_radr_ = 0.3;

  // Radar measurement noise standard deviation angle in rad
  std_radphi_ = 0.03;

  // Radar measurement noise standard deviation radius change in m/s
  std_radrd_ = 0.3;

  /**
  TODO:

  Complete the initialization. See ukf.h for other member properties.

  Hint: one or more values initialized above might be wildly off...
  */

  // Set to true in the first call of ProcessMeasurement
  is_initialized_ = false;

  // State dimension
  n_x_ = 5;

  // Augmented state dimension
  n_aug_ = 7;

  // Sigma point spreading parameter
  lambda_ = 3 - n_x_;

  // Predicted sigma points matrix
  Xsig_pred_ = MatrixXd(n_x_, 2 * n_aug_ + 1);

  // Weight vector
  weights_ = VectorXd(2 * n_aug_ + 1);

  // Current NIS for radar and laser
  NIS_radar_ = 0.0;
  NIS_laser_ = 0.0;

  // TIme when the state is true
  time_us_ = 0.0;
}

UKF::~UKF() {}

/**
 * @param {MeasurementPackage} meas_package The latest measurement data of
 * either radar or laser.
 */
void UKF::ProcessMeasurement(MeasurementPackage meas_package) {
  /**
  TODO:
  Complete this function! Make sure you switch between lidar and radar
  measurements.
  */
  
  // Skip the predict/update step if one of the sensor types is ignored.
  if ((meas_package.sensor_type_ == MeasurementPackage::RADAR && use_radar_) ||
      (meas_package.sensor_type_ == MeasurementPackage::LASER && use_laser_)) {
    

    // Initialize
    if (!is_initialized_) {

      // Initialize the first measurement
      x_ << 1, 1, 1, 1, 0.1;

      // Initialize the covariance matrix
      P_ << 0.15, 0, 0, 0, 0,
            0, 0.15, 0, 0, 0,
            0, 0, 1, 0, 0,
            0, 0, 0, 1, 0,
            0, 0, 0, 0, 1;

      // Initialize the time.
      time_us_ = meas_package.timestamp_;

	  // Laser
      if (meas_package.sensor_type_ == MeasurementPackage::LASER && use_laser_) {

        x_(0) = meas_package.raw_measurements_(0);
        x_(1) = meas_package.raw_measurements_(1);

      } // Radar
      else if (meas_package.sensor_type_ == MeasurementPackage::RADAR && use_radar_) {
        float rho = meas_package.raw_measurements_(0);
        float phi = meas_package.raw_measurements_(1);
        float rho_dot = meas_package.raw_measurements_(2);
        x_(0) = rho * cos(phi);
        x_(1) = rho * sin(phi);
      }

      // Finished initializing
      is_initialized_ = true;

      return;
    }

	// Predict
	
	// Compute the elapsed time.
    float dt = (meas_package.timestamp_ - time_us_) / 1000000.0;	// Units: seconds
    time_us_ = meas_package.timestamp_;

    Prediction(dt);

	// Update

    if (meas_package.sensor_type_ == MeasurementPackage::LASER) {
      UpdateLidar(meas_package);
    }
    else if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
      UpdateRadar(meas_package);
    }
  }
}

/**
 * Predicts sigma points, the state, and the state covariance matrix.
 * @param {double} delta_t the change in time (in seconds) between the last
 * measurement and this one.
 */
void UKF::Prediction(double delta_t) {
  /**
  TODO:

  Complete this function! Estimate the object's location. Modify the state
  vector, x_. Predict sigma points, the state, and the state covariance matrix.
  */
  
  // Mostly follow what was implemented in the lectures.
  
  // Create the sigma points
  
  // Create the sigma point matrix
  MatrixXd Xsig = MatrixXd(n_x_, 2 * n_x_ + 1);
  
  // Calculate the square root of P
  MatrixXd A = P_.llt().matrixL();
  
  // Set lambda for the non-augmented sigma points
  lambda_ = 3 - n_x_;
  
  // Set the first column for the sigma point matrix
  Xsig.col(0) = x_;
  
  // Set all of the remaining sigma points
  for (int i = 0; i < n_x_; i++) {
  	Xsig.col(i + 1) = x_ + sqrt(lambda_ + n_x_) * A.col(i);
  	Xsig.col(i + 1 + n_x_) = x_ - sqrt(lambda_ + n_x_) * A.col(i);
  }
  
  // Augment the sigma points
  
  // Augmented mean vector
  VectorXd x_aug = VectorXd(n_aug_);
  
  // Augmented state covariance matrix
  MatrixXd P_aug = MatrixXd(n_aug_, n_aug_); 
  
  // Sigma point matrix
  MatrixXd Xsig_aug = MatrixXd(n_aug_, 2 * n_aug_ + 1);
  
  // Lamda for augmented sigma points
  lambda_ = 3 - n_aug_;
  
  // Augmented man state
  x_aug.head(5) = x_;
  x_aug(5) = 0;
  x_aug(6) = 0;
  
  // Augmented covariance matrix
  P_aug.fill(0.0);
  P_aug.topLeftCorner(5, 5) = P_;
  P_aug(5, 5) = std_a_ * std_a_;
  P_aug(6, 6) = std_yawdd_ * std_yawdd_;
  
  // Square root matrix
  MatrixXd L = P_aug.llt().matrixL();
  
  // Augmented sigma points
  Xsig_aug.col(0) = x_aug;
  
  for (int i = 0; i < n_aug_; i++) {
  	Xsig_aug.col(i + 1) = x_aug + sqrt(lambda_ + n_aug_) * L.col(i);
  	Xsig_aug.col(i + 1 + n_aug_) = x_aug - sqrt(lambda_ + n_aug_) * L.col(i);
  }
  
  // Predict
  
  // Predict the sigma points.
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {
  	
  	// Assign variables
  	double p_x = Xsig_aug(0, i);
   	double p_y = Xsig_aug(1, i);
   	double v = Xsig_aug(2, i);
   	double yaw = Xsig_aug(3, i);
   	double yawd = Xsig_aug(4, i);
   	double nu_a = Xsig_aug(5, i);
  	double nu_yawdd = Xsig_aug(6, i);
  	
  	// Initialize predicted state values
  	double px_p, py_p;
  	
  	// Avoid the division by zero problem that can occur
  	if (fabs(yawd) > 0.001) {
  		px_p = p_x + v / yawd * (sin(yaw + yawd * delta_t) - sin(yaw));
  		py_p = p_y + v / yawd * (cos(yaw) - cos(yaw + yawd * delta_t));
  	} else {
  		px_p = p_x + v * delta_t * cos(yaw);
   		py_p = p_y + v * delta_t * sin(yaw);
  	}
  	
  	double v_p = v;
  	double yaw_p = yaw + yawd * delta_t;
  	double yawd_p = yawd;
  	
  	// Add noise to the state values
  	px_p = px_p + 0.5 * nu_a * delta_t * delta_t * cos(yaw);
  	py_p = py_p + 0.5 * nu_a * delta_t * delta_t * sin(yaw);
    v_p = v_p + nu_a * delta_t;
  	yaw_p = yaw_p + 0.5 * nu_yawdd * delta_t * delta_t;
  	yawd_p = yawd_p + nu_yawdd * delta_t;
  	
  	// Output the sigma points to the correct column
  	Xsig_pred_(0, i) = px_p;
  	Xsig_pred_(1, i) = py_p;
  	Xsig_pred_(2, i) = v_p;
  	Xsig_pred_(3, i) = yaw_p;
  	Xsig_pred_(4, i) = yawd_p;
  }
  
  // Convert predicted sigma points to mean and covariance
  
  // Set the weights
  double weight_0 = lambda_ / (lambda_ + n_aug_);
  weights_(0) = weight_0;
  
  for (int i = 1; i < 2 * n_aug_ + 1; i++) {
  	double weight = 0.5 / (n_aug_ + lambda_);
  	weights_(i) = weight;
  }
  
  // Predicted mean
  x_.fill(0.0);
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {
  	x_ = x_ + weights_(i) * Xsig_pred_.col(i);
  }
  
  // Predicted covariance matrix
  P_.fill(0.0);
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {
  	VectorXd x_diff = Xsig_pred_.col(i) - x_;
  	
  	// Normalize angles
    while (x_diff(3)> M_PI) x_diff(3) -= 2.*M_PI;
    while (x_diff(3)<-M_PI) x_diff(3) += 2.*M_PI;
  	
  	P_ = P_ + weights_(i) * x_diff * x_diff.transpose();
  }
}

/**
 * Updates the state and the state covariance matrix using a laser measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateLidar(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Use lidar data to update the belief about the object's
  position. Modify the state vector, x_, and covariance, P_.

  You'll also need to calculate the lidar NIS.
  */
  
  // Extract a measurement as a vector.
  VectorXd z = meas_package.raw_measurements_;
  
  // Set the dimension for measurement
  int n_z = 2;
  
  // Create sigma point matrix in measurement space.
  MatrixXd Zsig = MatrixXd(n_z, 2 * n_aug_ + 1);
  
  // Transform the sigma points into measurement space
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {
  	// Extract sigma points
  	double p_x = Xsig_pred_(0, i);
  	double p_y = Xsig_pred_(1, i);
  	
  	// Measurement model
  	Zsig(0, i) = p_x;
  	Zsig(1, i) = p_y;
  }
  
  // Predicted mean measurement
  VectorXd z_pred = VectorXd(n_z);
  z_pred.fill(0.0);
  
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {
  	z_pred = z_pred + weights_(i) * Zsig.col(i);
  }
  
  // Measurement covariance
  MatrixXd S = MatrixXd(n_z, n_z);
  S.fill(0.0);
  
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {
  	VectorXd z_diff = Zsig.col(i) - z_pred;
  	S = S + weights_(i) * z_diff * z_diff.transpose();
  }
  
  // Add measurement noise to the covariance matrix
  MatrixXd R = MatrixXd(n_z, n_z);
  
  R << std_laspx_ * std_laspx_, 0,
       0, std_laspy_ * std_laspy_;
  S = S + R;
  
  // Create cross correlation matrix
  MatrixXd Tc = MatrixXd(n_x_, n_z);

  Tc.fill(0.0);
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {

    // Residual
    VectorXd z_diff = Zsig.col(i) - z_pred;

    // Difference
    VectorXd x_diff = Xsig_pred_.col(i) - x_;

    Tc = Tc + weights_(i) * x_diff * z_diff.transpose();
  }

  // Kalman gain K;
  MatrixXd K = Tc * S.inverse();

  // Residual
  VectorXd z_diff = z - z_pred;

  // Calculate NIS
  NIS_laser_ = z_diff.transpose() * S.inverse() * z_diff;

  // Update the state and mean covariance matrices
  x_ = x_ + K * z_diff;
  P_ = P_ - K*S*K.transpose();
}

/**
 * Updates the state and the state covariance matrix using a radar measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateRadar(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Use radar data to update the belief about the object's
  position. Modify the state vector, x_, and covariance, P_.

  You'll also need to calculate the radar NIS.
  */
  
  // Extract the measurement as a vector.
  VectorXd z = meas_package.raw_measurements_;
  
  // Set the dimension for measurement.
  int n_z = 3;
  
  // Create sigma point matrix
  MatrixXd Zsig = MatrixXd(n_z, 2 * n_aug_ + 1);
  
  // Transform the sigma points into the measurement space
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {
  	double p_x = Xsig_pred_(0, i);
  	double p_y = Xsig_pred_(1, i);
  	double v = Xsig_pred_(2, i);
  	double yaw = Xsig_pred_(3, i);
  	double v1 = cos(yaw) * v;
  	double v2 = sin(yaw) * v;
  	
  	// r
  	Zsig(0, i) = sqrt(p_x * p_x + p_y * p_y);
	// phi
   	Zsig(1, i) = atan2(p_y, p_x);
   	//r_dot
   	Zsig(2, i) = (p_x * v1 + p_y * v2) / sqrt(p_x * p_x + p_y * p_y);
  }  
  
  // Predicted mean measurement
  VectorXd z_pred = VectorXd(n_z);
  z_pred.fill(0.0);
  
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {
  	z_pred = z_pred + weights_(i) * Zsig.col(i);
  }
  
  // Measurement covariance
  MatrixXd S = MatrixXd(n_z, n_z);
  S.fill(0.0);
  
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {
  	VectorXd z_diff = Zsig.col(i) - z_pred;
  	
  	  	// Normalize angles
    while (z_diff(1)> M_PI) z_diff(1) -= 2.*M_PI;
    while (z_diff(1)<-M_PI) z_diff(1) += 2.*M_PI;
  	
  	S = S + weights_(i) * z_diff * z_diff.transpose();
  }
  
  // Add measurement noise to the covariance matrix
  MatrixXd R = MatrixXd(n_z, n_z);
  
  R << std_radr_ * std_radr_, 0, 0,
  		0, std_radphi_ * std_radphi_, 0,
  		0, 0, std_radrd_ * std_radrd_;
  		
  S = S + R;
  
  // Create cross correlation matrix
  MatrixXd Tc = MatrixXd(n_x_, n_z);
  
  // UKF RADAR update
  Tc.fill(0.0);
  
  for (int i = 0; i < 2 * n_aug_ + 1; i++) {
  	
  	// Residual
  	VectorXd z_diff = Zsig.col(i) - z_pred;
  	
  	// Normalize angles
    while (z_diff(1)> M_PI) z_diff(1) -= 2.*M_PI;
    while (z_diff(1)<-M_PI) z_diff(1) += 2.*M_PI;
  	
  	// Difference
  	VectorXd x_diff = Xsig_pred_.col(i) - x_;
  	
  	  	// Normalize angles
    while (x_diff(3)> M_PI) x_diff(3) -= 2.*M_PI;
    while (x_diff(3)<-M_PI) x_diff(3) += 2.*M_PI;
  	
  	
  	Tc = Tc + weights_(i) * x_diff * z_diff.transpose();
  }
  
  // Residual
  VectorXd z_diff = z - z_pred;
  
  // Kalman gain
  MatrixXd K = Tc * S.inverse();
  
  // Normalize angles
  while (z_diff(1)> M_PI) z_diff(1) -= 2.*M_PI;
  while (z_diff(1)<-M_PI) z_diff(1) += 2.*M_PI;
  
  // Calculate NIS
  NIS_radar_ = z_diff.transpose() * S.inverse() * z_diff;
  
  // Update the state and mean covariance matrices
  x_ = x_ + K * z_diff;
  P_ = P_ - K * S * K.transpose();  
}