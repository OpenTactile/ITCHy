import sys
import os
#TODO: make independet from build path
sys.path.append(os.path.abspath('./build/lib.linux-x86_64-3.6/'))
import ITCHPy as itchpy
import time

import unittest


if __name__ == '__main__':
    itchy = itchpy.TactileMouseQuery()
    itchy.initialize()
    itchy.update()
    velocity = itchy.velocity()
    print('Velocity: {}'.format(velocity))
    print('Velocity-Length: {}'.format(velocity.length()))
    print('ButtonPressed: {}'.format(itchy.buttonPressed()))

    interactive_wait=1
    itchy.feedback(255,0,0)
    time.sleep(interactive_wait)
    itchy.feedback(0,255,0)
    time.sleep(interactive_wait)
    itchy.feedback(0,0,255)
    time.sleep(interactive_wait)
