## 5. Fluorescence Quantification

### 5.1 Background Correction (Self-Contained, Edge Cases)

#### Local Annulus Method (Default)

**Algorithm:**
```cpp
FluorescenceMetrics computeFluorescence(
    const cv::Mat& fluor_image,
    const std::vector<cv::Point>& droplet_contour,
    const cv::Mat& all_droplets_mask,  // Binary mask of all droplets (dilated)
    int annulus_width = 5
) {
    // Create droplet mask
    cv::Mat droplet_mask = cv::Mat::zeros(fluor_image.size(), CV_8U);
    cv::drawContours(droplet_mask, {droplet_contour}, 0, cv::Scalar(255), cv::FILLED);
    
    // Create annulus (dilated - original)
    cv::Mat dilated;
    cv::dilate(droplet_mask, dilated, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(annulus_width*2+1, annulus_width*2+1)));
    cv::Mat annulus = dilated - droplet_mask;
    
    // Exclude pixels inside other droplets
    cv::Mat valid_annulus;
    cv::bitwise_and(annulus, ~all_droplets_mask, valid_annulus);
    
    // Count valid annulus pixels
    int annulus_pixel_count = cv::countNonZero(valid_annulus);
    
    FluorescenceMetrics metrics;
    
    // Edge case: Annulus too small
    if (annulus_pixel_count < 10) {
        // Try expanding annulus
        annulus_width *= 2;  // e.g., 5 → 10
        cv::dilate(droplet_mask, dilated, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(annulus_width*2+1, annulus_width*2+1)));
        annulus = dilated - droplet_mask;
        cv::bitwise_and(annulus, ~all_droplets_mask, valid_annulus);
        annulus_pixel_count = cv::countNonZero(valid_annulus);
        
        if (annulus_pixel_count < 10) {
            // Fallback to global background (if available) or report failure
            metrics.bg_method = "failed";
            metrics.bg_corrected_mean = metrics.mean;  // Use raw mean
            metrics.sbr = NAN;
            return metrics;
        }
    }
    
    // Compute background mean
    cv::Scalar bg_mean = cv::mean(fluor_image, valid_annulus);
    float I_bg = static_cast<float>(bg_mean[0]);
    
    // Compute droplet metrics
    cv::Scalar droplet_mean = cv::mean(fluor_image, droplet_mask);
    double droplet_sum = cv::sum(fluor_image.mul(droplet_mask / 255.0))[0];  // Integrated intensity
    double min_val, max_val;
    cv::minMaxLoc(fluor_image, &min_val, &max_val, nullptr, nullptr, droplet_mask);
    
    metrics.mean = static_cast<float>(droplet_mean[0]);
    metrics.integrated = static_cast<float>(droplet_sum);
    metrics.min = static_cast<float>(min_val);
    metrics.max = static_cast<float>(max_val);
    metrics.bg_corrected_mean = metrics.mean - I_bg;
    metrics.sbr = metrics.mean / I_bg;
    metrics.bg_method = "local_annulus";
    
    // Edge case: Negative corrected mean
    if (metrics.bg_corrected_mean < 0) {
        metrics.bg_corrected_mean = 0.0f;  // Clamp to zero
        metrics.bg_corrected_negative_flag = true;
    }
    
    // Edge case: Saturated pixels
    float max_value = (fluor_image.depth() == CV_8U) ? 255.0f : 65535.0f;
    cv::Mat saturated_mask = (fluor_image == max_value);
    int saturated_count = cv::countNonZero(saturated_mask & droplet_mask);
    int droplet_pixel_count = cv::countNonZero(droplet_mask);
    if (saturated_count > droplet_pixel_count * 0.1) {  // >10% saturated
        metrics.saturated_flag = true;
    }
    
    return metrics;
}
```

#### Global Background ROI (Fallback)
- User draws background ROI in region with no droplets.
- Compute: $\bar{I}_{bg}^{global} = \text{mean}(\text{background ROI})$.
- Apply to all droplets: $\bar{I}_{corr} = \bar{I}_{droplet} - \bar{I}_{bg}^{global}$.

---
