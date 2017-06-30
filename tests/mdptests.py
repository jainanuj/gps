import os

statfile = "./res/stats/summarytable.txt"
statfileptr = open(statfile, "a")

statfileptr.write("Mdp_size, level_0_dims, level1_part_dims, part_avg_bytes, #updates, time_taken, Iter_Time, Queue_Time \n")
statfileptr.close()

mdps = ['grid', 'mcar']
states = [1000, 2000]
#base_parts = [500, 200]
#level1_parts = [ [5, 10],
#                [5, 10]]
base_parts = [500, 200, 100, 50, 40, 25, 20, 10]
level1_parts = [ [5, 10, 20, 25, 50, 100],
                 [5, 10, 20, 25, 50],
                 [5, 10, 20, 25],
                 [5, 10, 25],
                 [4, 5, 10],
                 [5],
                 [4, 5, 10],
                 [2, 5]]                      #10

i = 0

for mdp in mdps:
    for state in states:
        i = 0
        mdp_fn = "./mdps/"+mdp+'_'+str(state)+'x'+str(state)
        for base_part in base_parts:
            stp_fn = "./parts/"+str(state)+'-'+str(base_part)+'x'+str(base_part)
            for level1_size in level1_parts[i]:
                level1_stp = "./parts/"+str(base_part)+'-'+str(level1_size)+'x'+str(level1_size)
                result="./res/results_"+mdp+'_'+str(state)+'-'+str(base_part)+'-'+str(level1_size)
                cmd = "../solve/gps --epsilon=0.0001  --odcd_cache_fn_format=/tmp/odcd_cache-%d  --odcd_cache_size=-1  --mdp_fn=" \
                + mdp_fn + "  --stp_fn="+stp_fn\
                      +"  --sub_parts_st="+level1_stp+ "  --run_type=vi  --heat_metric=abs  --solver=r  --use_voting=0      --save_fn=" \
                +result+" --stat_fn="+statfile+"  --verbose=1"
                statfileptr = open(statfile, "a")
                statfileptr.write(mdp_fn + ', ' + str(base_part) + ', ' + str(level1_size) + ', ')
                statfileptr.close()
                print(cmd + '\n')
                os.system(cmd)
            i = i+1



