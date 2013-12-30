import os
def ensure_dir(f):
    d = os.path.dirname(f)
    print f
    print d
    if not os.path.exists(d):
    	#pass
        os.makedirs(d)