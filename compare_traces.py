import re

def extract_trace(line):
    line = line.strip()
    if len(lsplt := line.split('"')) != 1:
        operands = lsplt[-2].split(",")
        trace = line.split('"')[0].split(",")[0:-1]
        trace.append(operands)
    else:
        trace = line.split(',')
        trace[-1] = [trace[-1]]

    return trace
    
def match_format(to_match):
    matched = re.match(r"(.*)=\{(.*)\}", to_match)

    if matched is not None:
        return lambda x : (matched := re.match(r"(.*)=\{(.*)\}", to_match)).group(x)
    else:
        return lambda x : '0'

def parse_trace(header, trace):
    trace_parsed = {}
    trace_parsed['uuid'] = int(trace[header['uuid']])

    trace_parsed['PC'] = int(trace[header['PC']], 0)

    trace_parsed['opcode'] = trace[header['opcode']]

    trace_parsed['core_id'] = int(trace[header['core_id']])

    trace_parsed['warp_id'] = int(trace[header['warp_id']])

    trace_parsed['tmask'] = int(trace[header['tmask']])

    trace_parsed['destination'] = {'register' : match_format(trace[header['destination']])(1), 'value' : int(match_format(trace[header['destination']])(2), 0)}

    trace_parsed['operands'] = [
        {'register' : match_format(x)(1), 'value' : int(match_format(x)(2), 0)}
        for x in trace[header['operands']]
    ]

    return trace_parsed


def compare_traces(trace_1_filename, trace_2_filename):
    with open(trace_1_filename) as f:
        trace_1 = f.readlines()

    with open(trace_2_filename) as f:
        trace_2 = f.readlines()
    
    header = {e[1] : e[0] for e in enumerate(trace_1[0].strip().split(","))}

    for idx, line in enumerate(trace_2[1:]):
        trace_extracted = extract_trace(line)
        trace_2_extracted = extract_trace(trace_1[idx + 1])

        trace_parsed = parse_trace(header, trace_extracted)
        trace_2_parsed = parse_trace(header, trace_2_extracted)

        #print(f"{trace_parsed=}")

        if trace_parsed["opcode"] != trace_2_parsed["opcode"]:
            print("Different OPCODES!")
            print(line)
            break

        if len(trace_parsed['operands']) != len(trace_2_parsed['operands']):
            print("Wrong number of operands !")
            print(line)
            break
        else:
            for i, operand in enumerate(trace_parsed['operands']):
                if operand["register"] != trace_2_parsed['operands'][i]["register"]:
                    print(f"Wrong operand register {i}")
                    print(line)
                    break
                elif operand["value"] != trace_2_parsed['operands'][i]["value"]:
                    print(f"Wrong operand value, register {i}")
                    print(line)
                    break

        if trace_parsed["destination"]["register"] != trace_2_parsed["destination"]["register"]:
            print(f"Wrong destination register")
            print(line)
            break
        elif trace_parsed["destination"]["value"] != trace_2_parsed["destination"]["value"]:
            print(f"Wrong destination value")
            print(line)
            break

if __name__ == "__main__":
    trace_1 = input("Please write first file to check trace") 
    trace_2 = input("Please write second file to compare trace")
    
    compare_traces(trace_1, trace_2)
        # if any([trace_extracted[i] != trace_2_extracted[i] for i in range(len(khu_trace))]):
        #     print(f"Different: \n{trace_extracted=} \n{khu_trace=}")
        #     break

    
