# hwTarget.bit AXI Interfaces
## AXI Video Stream Interface

| Bytes |          Description          |
|:-----:|:-----------------------------:|
|   0   | LSB of Southwest Segment Data |
|   1   | MSB of Southwest Segment Data |
|   2   |    LSB of West Segment Data   |
|   3   |    MSB of West Segment Data   |
|   4   | LSB of Northwest Segment Data |
|   5   | MSB of Northwest Segment Data |
|   6   |   LSB of North Segment Data   |
|   7   |   MSB of North Segment Data   |

## AXI Lite Interface

| Address (Hex) |      Name      | Description                                                                         |
|:-------------:|:--------------:|-------------------------------------------------------------------------------------|
|      100      |     kirschL    | Large STD step size                                                                 |
|      104      |     kirschS    | Small STD step size                                                                 |
|      108      |     magThr     | Magnitude threshold value                                                           |
|      10C      |    max_addr    | Maximum read address of segment data (synonymous to max number of segments to read) |
|      110      | blankingClocks | The number of blanking cycles available                                             |
|      114      |  lineThreshold | The minimum line length to consider a segment                                       |
|      120      |  vCoefficients | Gaussian kernel vertical coefficients                                               |
|      140      |  hCoefficients | Gaussian kernel horizontal coefficients                                             |
