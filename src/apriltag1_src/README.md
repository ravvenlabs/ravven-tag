## quad_gradient.m
[Quad] = quad_gradient(image_blurred, image_gray, debug)
## Inputs
- image_blurred
-- This is a Length x Width x 1 image that is gray scale gaussian blurred image
- image_gray
-- This is a Length x Width x 1 image that is gray scale image
- debug
-- This is a flag that if it is not 0 will display a figure for each intermediate step
## Output
- Quads
-- This is an N x 8 array that has the potential quads from April Tags 1
## Description
Lorem Ipsum

## CalcEdges.m
[Edge] = CalcEdges(Magnitude, Direction, MagThr, height, width)
## Inputs
- Magnitude
-- This is a Length x Width x 1 image that is the gradient magnitude of the image
- Direction
-- This is a Length x Width x 1 image that is the gradient direction of the image
- MagThr
-- This is the minimum magnitude that a pixel will need to be checked
- height
-- This is the image height.
- width
-- This is the image width. 
## Output
- Edge
-- List of all the edges this is formatted [Cost, IdA, IdB, Px, Py]
## Description
Lorem Ipsum 

## MergeEdges.m
[Clusters] = CalcEdges(Edges, Magnitude, Direction)
## Inputs
- Edges
-- List of all the edges this is formatted [Cost, IdA, IdB, Px, Py]
- Magnitude
-- This is a Length x Width x 1 image that is the gradient magnitude of the image
- Direction
-- This is a Length x Width x 1 image that is the gradient direction of the image 
## Output
- Clusters
-- List of all the edges that are grouped together by group ID
-- Edges = N x 5 list of correlated edges
    1. Cost
	2. Ida (addr of current point) 
	3. Idb (addr of next connected point) 
	4. Point (x,y)
## Description
Lorem Ipsum 