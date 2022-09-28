from PIL import Image
import numpy as np
import matplotlib.pyplot as plt
img = Image.open("bitmap_font.png")
a = np.array(img,dtype=np.int32)
print(a.shape)
x = a.shape[0]
y = a.shape[1]
#a = np.resize(a, (x*y,4))
f = open("font.bin","wb")
l = []
for i in range(x):
    for j in range(y):
        #print(a[i][j])
        nl = list(a[i][j])
        #l.append(int(nl == [255,255,255,255]))
        l += [int(nl == [255,255,255,255])*240,int(nl == [255,255,255,255])*240,int(nl == [255,255,255,255])*255]#blue tinge
#plt.imshow(np.resize(np.array(l),(x,y)))
#plt.show()

f.write(bytearray(l))
f.close()
print("Done")

'''
print(l)
s = list(str(l))
print(s)
#s = ["{"] + l
s[0] = "{"
s[len(s)-1] = "}"

ns = ""
count = 0
for i in s:
    ns += i
    is_last_int = False
    try:
        g = int(ns[-1])
        is_last_int = True
    except: pass
    if count > 200 and is_last_int == False:
        ns += "\n"
        count = 0
    count += 1
'''
