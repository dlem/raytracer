require 'misc'
require 'cow_jump'

gr.option('--lores')

print('Six threads')
gr.option('-j 6')
cow_jump.render('threading.png')

print('Four threads')
gr.option('-j 4')
cow_jump.render('threading.png')

print('Two threads')
gr.option('-j 2')
cow_jump.render('threading.png')

print('One thread')
gr.option('-j 1')
cow_jump.render('threading.png')
