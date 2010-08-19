#!/usr/bin/python
import os
import sys

if ( len( sys.argv ) < 4 ):
   print "forkexec.py config_file np test_to_run"
   sys.exit(2)

config = sys.argv[1]
np = sys.argv[2]
test = sys.argv[3]
tmprev = test.split("/")
testname = tmprev[len(tmprev)-1]

for i in xrange(int(np)):
    rc = os.fork()
    if (rc > 0):
        os.environ['PAMI_SOCK_TASK'] = str(i)
        os.environ['PAMI_SOCK_SIZE'] = np
        os.environ['PAMI_UDP_CONFIG'] = config
        os.execlp(test,testname)
    elif (rc == 0):
        print "Started rank " + str(i)
    else:
        print "Failed to start rank " + str(i)


