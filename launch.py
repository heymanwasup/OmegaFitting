import subprocess,os,json,sys,argparse,re,inspect
sys.path.append('%s/../OmegaFitting'%(os.environ['PWD']))
import trans

condor = '''\
Universe   = vanilla
Executable = run_{1:}.sh
Log        = submit.log
{0:}
'''

queue = '''\
Arguments  = {0:} {1:} {2:} {3:} $(Process)
Output     = {5:}/condor_{2:}.{3:}.$(Process).out
Error      = {5:}/condor_{2:}.{3:}.$(Process).error
Queue {4:}
'''

executable = '''\
#! /bin/bash
source  /cvmfs/sft.cern.ch/lcg/views/LCG_101/x86_64-centos7-gcc11-opt/setup.sh
python ./launch.py --run ${1} ${2} ${3} ${4} ${5}
echo -e "END - ${1} ${2} ${3} ${4} ${5}"
'''

def submit(args):
    #default setup
    config = {
        'logDir' : './logs',
        'scan_per_queue' : 1,
    }

    with open(args[0],'r') as fp:
        config.update(json.load(fp)[args[1]])

    logDir = config['logDir'].format(key=args[1])
    os.system('mkdir -p %s'%(logDir))
    scan = config['scan']
    if isinstance(scan[0], str):
        N = int(len(scan) / config['scan_per_queue'])
    else:
        N = int(int((scan[1]-scan[0])/scan[2])  / config['scan_per_queue'])
    Queues = []
    for job in config['job']:
        for dataset in config['dataset']:
            q = queue.format(args[0],args[1],job,dataset,N,logDir)
            Queues.append(q)

    condor_str = condor.format('\n'.join(Queues),args[1])
    condor_file = 'submit_{0:}.condor'.format(args[1])
    with open(condor_file,'w') as fp:
        fp.write(condor_str)
    run_file = 'run_{0:}.sh'.format(args[1])
    with open(run_file,'w') as fp:
        fp.write(executable)
    os.system('chmod 777 {0:}'.format(run_file))
    os.system('condor_submit {0:}'.format(condor_file))

def run_queue(args):
    config = {
        'scan_per_queue' : 1,
    }
    with open(args[0],'r') as fp:
        config.update(json.load(fp)[args[1]])
    
    if config['scan_per_queue']==1:
        run(args, 0)
    else:
        job = args[2]
        dataset = args[3]
        process = args[4]
        for sub_num in range(config['scan_per_queue']):            
            print("\n",'='*20,'\n','Runing on {0:} {1:} process {2:} sub-{3:}'.format(dataset,job,process,sub_num),'\n','='*20,"\n")
            run(args,sub_num)
            

def run(args,sub_num):
    config = {
        'max_try' : 3,
        'start_bin' : 202,
        'end_bin' : 4357,
        'use_list' : 0,
        'scan_per_queue' : 1,
    }

    with open(args[0],'r') as fp:
        config.update(json.load(fp)[args[1]])

    job = args[2]
    dataset = args[3]
    scan_per_queue = config['scan_per_queue']
    q_num = int(args[4])

    if isinstance(config['scan'][0], str):
        N = len(config['scan'])
        process = (q_num % int(N/scan_per_queue))*scan_per_queue+sub_num
        scan = config['scan'][process]
    else:
        N = int((config['scan'][1]-config['scan'][0])/config['scan'][2])    
        process = (q_num % int(N/scan_per_queue))*scan_per_queue+sub_num
        step = config['scan'][2]
        scan = process * step + config['scan'][0]

    tag = '{0:}_{1:}'.format(job,dataset)
    # print (tag)
    if config['use_list']:
        if (not tag in config) or (not scan in config[tag]):
            print ('skip {0:} {1:}'.format(tag,scan))
            return

    kw = {
        'job' : job,
        'dataset' : dataset,
        'scan' : scan,
        'process' : process,
        'key' : args[1]
    }
    
    kw_extra = {}
    
    if 'keys' in config:
        for k,str_key in config['keys'].items():
            if str_key[:4] == 'def ':
                space = {}
                fname = re.split('def\ |\(',str_key)[1]
                exec(str_key,space)
                var_dict = dict.fromkeys(inspect.getfullargspec(space[fname])[0])
                for key in var_dict:
                    var_dict[key] = kw[key]
                kw_extra[k] = space[fname](**var_dict)                
            else:
                kw_extra[k] = str(str_key).format(**kw)

    kw.update(kw_extra)

    f = lambda key:str(config[key]).format(**kw)

    wiggle_file  = f('wiggle_file')
    wiggle_name  = f('wiggle_name')
    lm_file      = f('lm_file')
    lm_name      = f('lm_name')
    initial_file = f('initial_file')
    initial_name = f('initial_name')
    
    start_bin    = f('start_bin')
    end_bin      = f('end_bin')

    output_dir   = f('output_dir')
    value_dir    = f('value_dir')
    tag_out      = f('tag')

    mode = config['mode']
    max_try = config['max_try']
    
    value_of_func = config['value_of_func']    
    os.system('mkdir -p {0:}'.format(output_dir))    
    os.system('mkdir -p {0:}'.format(value_dir))

    fix = "None"
    if 'fix' in config:
        fix = ''
        for key,value in config['fix'].items():
            fix += '{0:} {1:} '.format(key,value)
    cmd = '../build/MAIN {0:} {1:} {2:} {3:} {4:} {5:} {6:} {7:} {8:} {9:} {10:} {11:} --fix {12:}'.format(
        wiggle_file,wiggle_name,lm_file,lm_name,initial_file,initial_name,output_dir,tag_out,mode,max_try,start_bin,end_bin,fix)

    print (cmd)

    os.system(cmd)

    res_name = '{0:}/result_{1:}_{2:}.root'.format(output_dir,value_of_func,tag_out)
    res_func_name = 'func_{0:}_{1:}'.format(value_of_func,tag_out)
    output_file = '{0:}/{1:}.root'.format(value_dir,tag_out)

    trans.extract(res_name,res_func_name,output_file,tag_out)

def fetch(args):
    with open(args[0],'r') as fp:
        config = json.load(fp)[args[1]]
    
    fetch_dir = './fetch/'
    if len(args)>2:
        fetch_dir = args[2]
    filter_str = '*'
    if len(args)>3:
        filter_str = args[3]

    os.system('mkdir -p {0:}'.format(fetch_dir))

    for job in config['job']:
        for dataset in config['dataset']:
            input_dir = config['output_dir'].format(job=job,dataset=dataset,key=args[1])
            basename = os.path.basename(input_dir.rstrip('/'))            
            cmd = 'hadd -j 8 -f {0:}/{1}.root {2:}/{3:}.root'.format(fetch_dir,basename,input_dir,filter_str)
            os.system(cmd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--submit',nargs='+')
    parser.add_argument('--run',nargs='+')
    parser.add_argument('--fetch',nargs='+')

    args = parser.parse_args()    
    if args.fetch!=None:
        fetch(args.fetch)
    elif args.submit!=None and args.run==None:
        submit(args.submit)
    elif args.submit==None and args.run!=None:
        run_queue(args.run)
    else:
        raise ValueError('use "python ./launch.py --submit/run/fetch ... "')