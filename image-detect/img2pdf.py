import cv2
import numpy as np
import os




def oneframe():
    #Read the list of directories and save it an array 
    entries = os.listdir('./processed_images')
    container = []
    print(entries)
    #Read the list and resize the images and finally append it to container 
    for entry in entries:
        image = cv2.imread('./processed_images/' + entry)
        image = cv2.resize(image, (0,0), fx=0.5, fy=0.5)
        container.append(image)

    #Horizontal Concatenation and display image.
    h_concat = cv2.hconcat(container)

    cv2.imshow("Concatenated Images", h_concat)
    cv2.waitKey(0)
    cv2.destroyAllWindows()


# #Read the list of directories and save it an array 
# entries = os.listdir('./processed_images')
# container = []
# print(entries)
# #Read the list and resize the images and finally append it to container 
# for entry in entries:
#     image = cv2.imread('./processed_images/' + entry)
#     image = cv2.resize(image, (0,0), fx=0.5, fy=0.5)
#     container.append(image)

# #Horizontal Concatenation and display image.
# h_concat = cv2.hconcat(container)

# cv2.imshow("Concatenated Images", h_concat)
# cv2.waitKey(0)
# cv2.destroyAllWindows()


