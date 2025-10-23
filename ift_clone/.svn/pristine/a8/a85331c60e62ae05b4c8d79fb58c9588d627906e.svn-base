import cv2
import numpy as np
import matplotlib.pyplot as plt
from scipy.ndimage import map_coordinates
from scipy.interpolate import griddata
import os
import sys


class FingerprintRegistration:
    def __init__(self):
        """Initialize fingerprint registration class."""
        self.orb = cv2.ORB_create(5000)
        self.bf = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=True)
    
    def enhance_fingerprint(self, image):
        """
        Enhance fingerprint image using basic preprocessing.
        """
        # Convert to grayscale if needed
        if len(image.shape) == 3:
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        else:
            gray = image.copy()
        
        # Apply Gaussian blur to reduce noise
        blurred = cv2.GaussianBlur(gray, (5, 5), 0)
        
        # Apply adaptive histogram equalization
        clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
        enhanced = clahe.apply(blurred)
        
        # Apply binary threshold
        _, binary = cv2.threshold(enhanced, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
        
        return enhanced, binary
    
    def detect_and_match_features(self, img1, img2):
        """
        Detect ORB features and match them between two images.
        """
        # Detect keypoints and descriptors
        kp1, desc1 = self.orb.detectAndCompute(img1, None)
        kp2, desc2 = self.orb.detectAndCompute(img2, None)
        
        if desc1 is None or desc2 is None:
            return None, None, []
        
        # Match descriptors
        matches = self.bf.match(desc1, desc2)
        matches = sorted(matches, key=lambda x: x.distance)
        
        # Filter good matches (top 70%)
        good_matches = matches[:int(len(matches) * 0.7)]
        
        return kp1, kp2, good_matches
    
    def compute_homography(self, kp1, kp2, matches, min_matches=20):
        """
        Compute homography matrix from matched keypoints.
        """
        if len(matches) < min_matches:
            return None, None
        
        # Extract coordinates of matched points
        src_pts = np.float32([kp1[m.queryIdx].pt for m in matches]).reshape(-1, 1, 2)
        dst_pts = np.float32([kp2[m.trainIdx].pt for m in matches]).reshape(-1, 1, 2)
        
        # Compute homography using RANSAC
        homography, mask = cv2.findHomography(src_pts, dst_pts, 
                                            cv2.RANSAC, 5.0)
        
        return homography, mask
    
    def create_dense_deformation_field(self, kp1, kp2, matches, img_shape, grid_spacing=5):
        """
        Create dense deformation field from sparse keypoint matches.
        """
        height, width = img_shape[:2]
        
        # Extract matched point coordinates
        src_points = np.array([kp1[m.queryIdx].pt for m in matches])
        dst_points = np.array([kp2[m.trainIdx].pt for m in matches])
        
        # Calculate displacement vectors
        displacements = dst_points - src_points
        
        # Create regular grid
        x_grid = np.arange(0, width, grid_spacing)
        y_grid = np.arange(0, height, grid_spacing)
        xx, yy = np.meshgrid(x_grid, y_grid)
        grid_points = np.column_stack([xx.ravel(), yy.ravel()])
        
        # Interpolate displacements to grid
        if len(src_points) > 3:  # Need at least 3 points for interpolation
            dx_interp = griddata(src_points, displacements[:, 0], grid_points, 
                               method='linear', fill_value=0)
            dy_interp = griddata(src_points, displacements[:, 1], grid_points, 
                               method='linear', fill_value=0)
        else:
            dx_interp = np.zeros(len(grid_points))
            dy_interp = np.zeros(len(grid_points))
        
        # Reshape to grid format
        dx_grid = dx_interp.reshape(len(y_grid), len(x_grid))
        dy_grid = dy_interp.reshape(len(y_grid), len(x_grid))
        
        # Interpolate to full image resolution
        dx_full = cv2.resize(dx_grid, (width, height))
        dy_full = cv2.resize(dy_grid, (width, height))
        
        # Create deformation field (y, x, 2) format
        deformation_field = np.zeros((height, width, 2), dtype=np.float32)
        deformation_field[:, :, 0] = dx_full  # x displacement
        deformation_field[:, :, 1] = dy_full  # y displacement
        
        return deformation_field
    
    def apply_deformation_field(self, image, deformation_field):
        """
        Apply deformation field to transform an image.
        """
        height, width = image.shape[:2]
        
        # Create coordinate grids
        y_coords, x_coords = np.mgrid[0:height, 0:width]
        
        # Apply deformation
        new_x = x_coords + deformation_field[:, :, 0]
        new_y = y_coords + deformation_field[:, :, 1]
        
        # Ensure coordinates are within bounds
        new_x = np.clip(new_x, 0, width - 1)
        new_y = np.clip(new_y, 0, height - 1)
        
        # Apply transformation
        if len(image.shape) == 2:
            registered_image = map_coordinates(image, [new_y, new_x], 
                                             order=1, mode='nearest')
        else:
            registered_image = np.zeros_like(image)
            for i in range(image.shape[2]):
                registered_image[:, :, i] = map_coordinates(image[:, :, i], 
                                                          [new_y, new_x], 
                                                          order=1, mode='nearest')
        
        return registered_image.astype(image.dtype)
    
    def register_fingerprints(self, fixed_image, moving_image):
        """
        Main function to register two fingerprint images and return registered image and deformation field.
        
        Args:
            fixed_image: Reference fingerprint image
            moving_image: Image to be registered to the reference
            
        Returns:
            registered_image: Registered moving image
            deformation_field: Dense deformation field
            homography: Homography matrix (if computed)
        """
        # Enhance fingerprint images
        fixed_enhanced, fixed_binary = self.enhance_fingerprint(fixed_image)
        moving_enhanced, moving_binary = self.enhance_fingerprint(moving_image)
        
        # Detect and match features
        kp1, kp2, matches = self.detect_and_match_features(moving_enhanced, fixed_enhanced)
        
        if not matches:
            print("No matches found between images!")
            return None, None, None
        
        print(f"Found {len(matches)} good matches")
        
        # Compute homography for global registration
        homography, mask = self.compute_homography(kp1, kp2, matches)
        
        if homography is None:
            print("Could not compute homography!")
            return None, None, None
        
        # Create dense deformation field from matches
        deformation_field = self.create_dense_deformation_field(
            kp1, kp2, matches, fixed_image.shape
        )
        
        # Apply deformation to register the image
        registered_image = self.apply_deformation_field(moving_image, deformation_field)
        
        return registered_image, deformation_field, homography
    
    def visualize_results(self, fixed_image, moving_image, registered_image, 
                         deformation_field, kp1=None, kp2=None, matches=None):
        """
        Visualize registration results.
        """
        fig, axes = plt.subplots(2, 3, figsize=(15, 10))
        
        # Original images
        axes[0, 0].imshow(fixed_image, cmap='gray')
        axes[0, 0].set_title('Fixed Image')
        axes[0, 0].axis('off')
        
        axes[0, 1].imshow(moving_image, cmap='gray')
        axes[0, 1].set_title('Moving Image')
        axes[0, 1].axis('off')
        
        axes[0, 2].imshow(registered_image, cmap='gray')
        axes[0, 2].set_title('Registered Image')
        axes[0, 2].axis('off')
        
        # Deformation field visualization
        h, w = deformation_field.shape[:2]
        y, x = np.mgrid[0:h:10, 0:w:10]
        dx = deformation_field[::10, ::10, 0]
        dy = deformation_field[::10, ::10, 1]
        
        axes[1, 0].imshow(fixed_image, cmap='gray', alpha=0.7)
        axes[1, 0].quiver(x, y, dx, dy, angles='xy', scale_units='xy', scale=1, color='red')
        axes[1, 0].set_title('Deformation Field')
        axes[1, 0].axis('off')
        
        # Deformation magnitude
        deformation_magnitude = np.sqrt(deformation_field[:, :, 0]**2 + deformation_field[:, :, 1]**2)
        axes[1, 1].imshow(deformation_magnitude, cmap='jet')
        axes[1, 1].set_title('Deformation Magnitude')
        axes[1, 1].axis('off')
        
        # Difference image
        if fixed_image.shape == registered_image.shape:
            diff_image = np.abs(fixed_image.astype(float) - registered_image.astype(float))
            axes[1, 2].imshow(diff_image, cmap='gray')
            axes[1, 2].set_title('Difference Image')
            axes[1, 2].axis('off')
        
        plt.tight_layout()
        plt.show()

# Example usage
def main():
    # Initialize registration class
    registrator = FingerprintRegistration()
    
    if (len(sys.argv) != 3):
        print("python register.py <P1> <P2>")
        print("P1: fixed fingerprint")
        print("P2: moving fingerprint")
        exit()

    fixed_name   = sys.argv[1]    
    moving_name  = sys.argv[2]    

    # Load fingerprint images (replace with your image paths)
    fixed_image  = cv2.imread(fixed_name, cv2.IMREAD_GRAYSCALE)
    moving_image = cv2.imread(moving_name, cv2.IMREAD_GRAYSCALE)
    
    # For demonstration, create synthetic fingerprint-like images
    # In practice, replace this with actual fingerprint loading
    #fixed_image = np.random.randint(0, 255, (256, 256), dtype=np.uint8)
    #moving_image = np.roll(fixed_image, (10, 15), axis=(0, 1))  # Simulate displacement
    
    print("Starting fingerprint registration...")
    
    # Perform registration
    registered_image, deformation_field, homography = registrator.register_fingerprints(
        fixed_image, moving_image
    )
    
    if registered_image is not None:
        print("Registration successful!")
        print(f"Deformation field shape: {deformation_field.shape}")
        print(f"Homography matrix:\n{homography}")
        
        # Visualize results
        registrator.visualize_results(fixed_image, moving_image, registered_image, 
                                    deformation_field)
        
        # Save results
        # cv2.imwrite('registered_fingerprint.jpg', registered_image)
        # np.save('deformation_field.npy', deformation_field)
        
    else:
        print("Registration failed!")

if __name__ == "__main__":
    main()
