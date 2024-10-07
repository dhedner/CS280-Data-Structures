Water retained by terrain.

Imaging terrain which is made of columns (like bar graph): 
1-dimensional case:
8     +-+        
7     | |   +-+   
6 +-+ | |   | |   
5 | | | +-+ | |  
4 | | | | | | |  
3 | +-+ | | | |  
2 | | | | | | +-+
1 | | | | +-+ | |
  +-+-+-+-+-+-+-+
   6 3 8 5 1 7 2  <-- heights

If it is raining over the whole terrain - how much water will be retained?

8     +-+            
7     | |w w+-+        
6 +-+w| |w w| |   
5 | |w| +-+w| |  
4 | |w| | |w| |  
3 | +-+ | |w| |  
2 | | | | |w| +-+
1 | | | | +-+ | |
  +-+-+-+-+-+-+-+
   6 3 8 5 1 7 2  <-- heights

Problem: given an array of heights calculate the amount of retained water.
Solution:
add 0 to the left and right of the array
scan left to right and keep track of the max level on the left (left runoff level)
during the scan think that we allow water to escape left only:
    init runoff to 0 (edge of the terrain)                                          |water |
    6 - runoff 0, since 6>0 there is no water on top of this column                 |   0  | update runoff to 6
    3 - runoff 6, since 3<6 there is maximum 3 units of water on top of this column |   3  | update runoff to 6 ( water level + column height)
    8 - runoff 6, since 8>6 there is no water on top of this column                 |   0  | update runoff to 8 ( max old cutoff and height)
    5 - runoff 8, since 5<8 there is maximum 3 units of water on top of this column |   3  | 
    1 - runoff 8, since 1<8 there is maximum 7 units of water on top of this column |   7  | 
    7 - runoff 8, since 7<8 there is maximum 1 units of water on top of this column |   1  | 
    2 - runoff 8, since 2<8 there is maximum 6 units of water on top of this column |   6  | 
Final picture after 1st scan:
8     +-+w w w w 
7     | |w w+-+w 
6 +-+w| |w w| |w  
5 | |w| +-+w| |w 
4 | |w| | |w| |w 
3 | +-+ | |w| |w  
2 | | | | |w| +-+
1 | | | | +-+ | |
  +-+-+-+-+-+-+-+
   6 3 8 5 1 7 2  <-- heights

Now scan right to left allowing water to escape to the right only
    init runoff to 0 (edge of the terrain)
    inequality looks at height+water ?<>? runoff 
        height+water < runoff ( water stays)
        height+water > runoff ( water has to be updated - some will definitely escape)
            update is water=min( runoff-height, 0 )
    2 - runoff 0, since 2+6>0 water update = min(0-2,0)= 0                              0   update runoff to 2
    7 - runoff 2, since 7+1>2 water update = min(2-7,0)= 0                              0   update runoff to 7
    1 - runoff 7, since 1+7>7 water update = min(7-1,0)= 6                              6   
    5 - same as above, new water level = 7 - 5                                          2
    8 - runoff 7, since 8+0>7 water update = min(7-8,0)= 0                              0   update runoff to 8
    3 - 3+3<8, no changes to water level                                                3
    6 - 6+0<8, no changes to water level                                                0
Final picture after 2nd scan:
8     +-+. . . . 
7     | |w w+-+. 
6 +-+w| |w w| |.  
5 | |w| +-+w| |. 
4 | |w| | |w| |.    <- dots show water removed by second scan.
3 | +-+ | |w| |.  
2 | | | | |w| +-+
1 | | | | +-+ | |
  +-+-+-+-+-+-+-+
   6 3 8 5 1 7 2  <-- heights

You are to solve 2 dimensional case of this problem:

Problem: given a 2-dimensional array of heights calculate the amount of retained water.

Example:
+----+----+----+----+----+
|h=7 |h=7 |h=4 |h=2 |h=4 |    
|    |    |    |    |    |    
+----+----+----+----+----+
|h=7 |h=0 |h=4 |h=2 |h=4 |    
|    |    |    |    |    |    
+----+----+----+----+----+
|h=7 |h=3 |h=4 |h=2 |h=1 |    
|    |    |    |    |    |    
+----+----+----+----+----+
|h=7 |h=7 |h=4 |h=4 |h=4 |    
|    |    |    |    |    |    
+----+----+----+----+----+

Example solution:
Obvious step - edges, keeping track of cutoffs and water:
+----+----+----+----+----+
|h=7 |h=7 |h=4 |h=2 |h=4 |    
|7 0 |7 0 |4 0 |2 0 |4 0 |    cutoff    waterlevel
+----+----+----+----+----+
|h=7 |h=0 |h=4 |h=2 |h=4 |    
|7 0 |    |    |    |4 0 |    
+----+----+----+----+----+
|h=7 |h=3 |h=4 |h=2 |h=1 |    
|7 0 |    |    |    |1 0 |    
+----+----+----+----+----+
|h=7 |h=7 |h=4 |h=4 |h=4 |    
|7 0 |7 0 |4 0 |4 0 |4 0 |    
+----+----+----+----+----+
Now instead of thinking that high water is escaping if lower column is a neighbor (see note below)
think similar to the 1-dimensional case that low water sucks high water out
+----+----+----+----+----+
|h=7 |h=7 |h=4 |h=2 |h=4 |    
|7 0 |7|0 |4|0 |2 0 |4 0 |    cutoff    waterlevel
+----+-V--+-V--+----+----+    arrow shows which lower level cell updated the neighbor
|h=7 |h=0 |h=4 |h=2 |h=4 |    
|7 0 |7 7 |4 0 |    |4 0 |    cells 1,1 and 1,2 (0-based indexing) updated
+----+----+----+----+----+
|h=7 |h=3 |h=4 |h=2 |h=1 |    
|7 0 |    |    |    |1 0 |    
+----+----+----+----+----+
|h=7 |h=7 |h=4 |h=4 |h=4 |    
|7 0 |7 0 |4 0 |4 0 |4 0 |    
+----+----+----+----+----+

Note: water cannot escape through diagonals (that's why 4 neighbors).

+----+----+-----+----+----+
|h=7 |h=7 | h=4 |h=2 |h=4 |    
|7 0 |7 0 | 4 0 |2 0 |4 0 |    cutoff    waterlevel
+----+----+-----+----+----+    arrow shows which lower level cell updated the neighbor
|h=7 |h=0 | h=4 |h=2 |h=4 |    
|7 0 |4 4 <-4 0 |    |4 0 |    notice updated cell may be updated again if
+----+----+-----+----+----+    on of the neighbors was updated
|h=7 |h=3 | h=4 |h=2 |h=1 |    
|7 0 |    |     |    |1 0 |    cell 1,1 was updated again
+----+----+-----+----+----+
|h=7 |h=7 | h=4 |h=4 |h=4 |    
|7 0 |7 0 | 4 0 |4 0 |4 0 |    
+----+----+-----+----+----+

Idea: ALL perimeter cells are trivial and they start cascading updates.

Code:
a queue of cells that were updated and may be used to update neighbors
    pop element
    check if you can update any of its 4 neighbors, if yes - update and insert neighbor into queue for cascading effect

Note that 1-dimensional solution does exactly the same:
    perimeter - 2 cells,
    starting with perimeter update nodes, since it's 1-dimensional we do not need a queue - just scan
