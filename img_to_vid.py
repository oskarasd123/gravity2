import cv2
import glob

path = "img/*.pgm"

filenames = sorted(glob.glob(path))

# Read the first image to determine the width and height
first_frame = cv2.imread(filenames[0])
height, width, layers = first_frame.shape

# Define the codec and create VideoWriter object
# The 'XVID' is a common codec. You can also use 'MP4V' for .mp4 format.
fourcc = cv2.VideoWriter_fourcc(*'MP4V')
out = cv2.VideoWriter('output.mp4', fourcc, 60.0, (width, height))

# Loop through all the files
for i in range(len(filenames)):
    if (i%1 != 0):
        continue
    img = cv2.imread(filenames[i])
    out.write(img)  # Write the frame to the video

# Release everything when the job is finished
out.release()

print("Video creation complete.")
