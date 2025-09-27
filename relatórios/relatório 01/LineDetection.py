import cv2
import os
import array
import numpy as np
from scipy.interpolate import griddata
from skimage.morphology import skeletonize

os.chdir(r"C:\Users\acyrm\Documents\UTFPR\7Periodo\OpenCV\Images")

###LINE DETECTION
################################################################
def Line_detection (img):
    line_image = np.copy(img) * 0  # creating a blank to draw lines on
    height, width = img.shape[:2]
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    blur_gray = cv2.GaussianBlur(gray, (5,5), 0)

    #Edge detection
    low_threashold = 50
    high_threashold = 150
    edges = cv2.Canny(blur_gray, low_threashold, high_threashold)

    #Get lines
    rho = 1  # distance resolution in pixels of the Hough grid
    theta = np.pi / 180  # angular resolution in radians of the Hough grid
    threshold = 15  # minimum number of votes (intersections in Hough grid cell)
    min_line_length = 50  # minimum number of pixels making up a line
    max_line_gap = 6  # maximum gap in pixels between connectable line segments

    # Run Hough on edge detected image
    # Output "lines" is an array containing endpoints of detected line segments
    lines = cv2.HoughLinesP(edges, rho, theta, threshold, np.array([]),
                        min_line_length, max_line_gap)
    #x1, y1, x2, y2
    return lines


###LINE FILTER
################################################################
def Line_filter(lines):
    #Encontra o numero de linhas validas
    num_valid_lines = 0
    for line in lines:
        for x1,y1,x2,y2 in line:
            if (abs(x1 - x2) <= abs(y1 - y2))  and (y1 < int(height/2) or y2 < int(height/2)):
                num_valid_lines += 1
    #Cria novos vetores com o número de linhas encontrado
    lines_filtered = np.zeros((num_valid_lines, 4))
    lines_filtered_x1 = np.zeros(num_valid_lines)
    lines_filtered_y1 = np.zeros(num_valid_lines)
    lines_filtered_x2 = np.zeros(num_valid_lines)
    lines_filtered_y2 = np.zeros(num_valid_lines)
    result = np.zeros((num_valid_lines, 4))
    num_valid_lines = 0
    #Adiciona os valores validos aos novos vetores
    for line in lines:
        for x1,y1,x2,y2 in line:
            if (abs(x1 - x2) <= abs(y1 - y2))  and (y1 < int(height/2) or y2 < int(height/2)):
                lines_filtered[num_valid_lines]=[x1, y1, x2, y2]
                lines_filtered_x1[num_valid_lines] = x1
                lines_filtered_y1[num_valid_lines] = y1
                lines_filtered_x2[num_valid_lines] = x2
                lines_filtered_y2[num_valid_lines] = y2
                num_valid_lines += 1
    #Organiza os vetores para coordenadas especificas
    lines_filtered_x1.sort()
    lines_filtered_y1.sort()
    lines_filtered_x2.sort()
    lines_filtered_y2.sort()
    #Filtra esses vetores para retirar valores similares
    threshold = 5
    lines_filtered_x1 = lines_filtered_x1[np.r_[True, np.abs(np.diff(lines_filtered_x1))>=threshold]]
    lines_filtered_y1 = lines_filtered_y1[np.r_[True, np.abs(np.diff(lines_filtered_y1))>=threshold]]
    lines_filtered_x2 = lines_filtered_x2[np.r_[True, np.abs(np.diff(lines_filtered_x2))>=threshold]]
    lines_filtered_y2 = lines_filtered_y2[np.r_[True, np.abs(np.diff(lines_filtered_y2))>=threshold]]
    set_x1 = set(lines_filtered_x1)
    set_y1 = set(lines_filtered_y1)
    set_x2 = set(lines_filtered_x2)
    set_y2 = set(lines_filtered_y2)
    #Retira linhas que não possuam coordenadas em nenhum dos vetores separado
    print (type(lines[0, 0]))
    print (type(lines_filtered[0, 0]))
    i = 0
    for line in lines_filtered :
        for x1, y1, x2, y2 in line :
            if ((x1 in set_x1) and (y1 in set_y1) and (x2 in set_x2) and (y2 in set_y2)):

                i += 1
    print (result)
    return lines_filtered


###IMAGE WARPING
################################################################
def Linear_warp (img, lines):
    grid_x, grid_y = np.mgrid[0:49:50j, 0:799:800j]

    source = np.array([ [80,35], [58,157], [43, 284], [34, 406], [33, 528], [38, 648], [48, 764], [66, 880], [88, 989],
                        [143,72], [121,186], [106, 298], [99, 410], [96, 522], [98, 633], [107, 739], [125, 844], [145, 948]])
    destination = np.array([[0,0], [0,99], [0,199], [0, 299], [0,399], [0, 499], [0,599], [0, 699], [0,799],
                            [49,0], [49,99], [49,199], [49, 299], [49,399], [49, 499], [49,599], [49, 699], [49,799]])

    grid_z = griddata(destination, source, (grid_x, grid_y), method='cubic')
    map_x = np.append([], [ar[:,1] for ar in grid_z]).reshape(50,800)
    map_y = np.append([], [ar[:,0] for ar in grid_z]).reshape(50,800)
    map_x_32 = map_x.astype('float32')
    map_y_32 = map_y.astype('float32')

    warped = cv2.remap(img, map_x_32, map_y_32, cv2.INTER_CUBIC)




###MAIN CODE
################################################################
img = cv2.imread('RealScale.jpg')
height, width = img.shape[:2]
lines = Line_detection(img)
lines_filtered = Line_filter(lines)
line_image = np.copy(img) * 0  # creating a blank to draw lines on

for line in lines:
    for x1,y1,x2,y2 in line:
        if (abs(x1 - x2) <= abs(y1 - y2))  and (y1 < int(height/2) or y2 < int(height/2)):
            cv2.line(line_image,(x1,y1),(x2,y2),(255,0,0),5)

#Draw the lines on the  image
lines_edges = cv2.addWeighted(img, 0.8, line_image, 1, 0)
cv2.imwrite('Detection.png', lines_edges)
cv2.imshow('Test', lines_edges)
cv2.waitKey(0)
cv2.destroyAllWindows()
