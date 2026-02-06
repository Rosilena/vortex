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


if __name__ == "__main__":
    with open("dogfood_no_khu.csv") as f:
        dogfood_no_khu = f.readlines()

    with open("dogfood_khu.csv") as f:
        dogfood_khu = f.readlines()
    
    header = {e[1] : e[0] for e in enumerate(dogfood_no_khu[0].strip().split(","))}

    for idx, line in enumerate(dogfood_khu[1:]):
        khu_trace = extract_trace(line)
        no_khu_trace = extract_trace(dogfood_no_khu[idx + 1])

        khu_trace_parsed = parse_trace(header, khu_trace)
        no_khu_trace_parsed = parse_trace(header, no_khu_trace)

        #print(f"{khu_trace_parsed=}")

        if khu_trace_parsed["opcode"] != no_khu_trace_parsed["opcode"]:
            print("Different OPCODES!")
            print(line)
            break

        if len(khu_trace_parsed['operands']) != len(no_khu_trace_parsed['operands']):
            print("Wrong number of operands !")
            print(line)
            break
        else:
            for i, operand in enumerate(khu_trace_parsed['operands']):
                if operand["register"] != no_khu_trace_parsed['operands'][i]["register"]:
                    print(f"Wrong operand register {i}")
                    print(line)
                    break
                elif operand["value"] != no_khu_trace_parsed['operands'][i]["value"]:
                    print(f"Wrong operand value, register {i}")
                    print(line)
                    break

        if khu_trace_parsed["destination"]["register"] != no_khu_trace_parsed["destination"]["register"]:
            print(f"Wrong destination register")
            print(line)
            break
        elif khu_trace_parsed["destination"]["value"] != no_khu_trace_parsed["destination"]["value"]:
            print(f"Wrong destination value")
            print(line)
            break
    
        
        # if any([khu_trace[i] != no_khu_trace[i] for i in range(len(khu_trace))]):
        #     print(f"Different: \n{khu_trace=} \n{khu_trace=}")
        #     break

    