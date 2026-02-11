// Copyright © 2019-2023
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

`include "VX_define.vh"


module VX_khu_int import VX_gpu_pkg::*; import VX_khu_sbox_pkg::*; #(
    parameter `STRING INSTANCE_ID = "",
    parameter NUM_LANES = 1
) (
    input wire              clk,
    input wire              reset,

    // Inputs
    VX_execute_if.slave     execute_if,

    // Outputs
    VX_result_if.master     result_if
);

    `UNUSED_SPARAM (INSTANCE_ID)
    //localparam LANE_BITS      = `CLOG2(NUM_LANES);
    //localparam LANE_WIDTH     = `UP(LANE_BITS);
    localparam PID_BITS       = `CLOG2(`NUM_THREADS / NUM_LANES);
    localparam PID_WIDTH      = `UP(PID_BITS);
    //localparam SHIFT_IMM_BITS = `CLOG2(`XLEN);

    `UNUSED_VAR (execute_if.data.rs3_data)

    reg  [NUM_LANES-1:0][`XLEN-1:0] zbkb_result;
    reg  [NUM_LANES-1:0][`XLEN-1:0] zbkc_zbkx_result;
    reg  [NUM_LANES-1:0][`XLEN-1:0] zknd_zkne_result;
    reg  [NUM_LANES-1:0][`XLEN-1:0] zknh_result;

    reg [NUM_LANES-1:0][`XLEN-1:0] khu_result;
    wire [NUM_LANES-1:0][`XLEN-1:0] khu_result_r;

/*
`ifdef XLEN_64
    wire is_khu_w = execute_if.data.op_args.khu.is_w;
`else
    wire is_khu_w = 0;
`endif
*/
    wire [INST_KHU_BITS-1:0] khu_op = INST_KHU_BITS'(execute_if.data.op_type);

    wire [NUM_LANES-1:0][`XLEN-1:0] khu_in1 = execute_if.data.rs1_data;
    wire [NUM_LANES-1:0][`XLEN-1:0] khu_in2 = execute_if.data.rs2_data;
    reg [NUM_LANES-1:0][`CLOG2(`XLEN) - 1 : 0] shamt;
    reg [NUM_LANES-1:0][`CLOG2(`XLEN) - 1 : 0] shamt_imm;
    reg [NUM_LANES-1:0][`XLEN-1:0] result; //for zbkc and zbkx instruction


    `ifdef XLEN_64
    reg [NUM_LANES-1:0][4 : 0] shamt_xlen;
    reg [NUM_LANES-1:0][4 : 0] shamt_imm_xlen;
    reg [NUM_LANES-1:0][31 : 0] khu_in1_xlen;
    reg [NUM_LANES-1:0][31 : 0] result_w;
    `endif 


    //for zknd
    `ifndef XLEN_64 //only riscv 32
    reg [1:0] bs = execute_if.data.op_args.khu.bs; //funct7[6:5]
    reg [NUM_LANES-1:0][4:0] shamt_aes;
    reg [NUM_LANES-1:0][7:0] si;
    reg [NUM_LANES-1:0][31:0] tmp;
    reg [NUM_LANES-1:0][31:0] so_32;
    reg [NUM_LANES-1:0][7:0] so_8;
    reg [NUM_LANES-1:0][31:0] mixed;
    reg [NUM_LANES-1:0][31:0] result_aes;
    `endif 
    `ifdef XLEN_64
    reg [NUM_LANES-1:0][63:0] sr;
    reg [NUM_LANES-1:0][63:0] sb;
    reg [NUM_LANES-1:0][31:0] w0;
    reg [NUM_LANES-1:0][31:0] w1;
    reg [NUM_LANES-1:0][31:0] tmp1;
    reg [NUM_LANES-1:0][31:0] tmp2;
    reg [NUM_LANES-1:0][31:0] tmp3;
    reg [NUM_LANES-1:0][31:0] rc;
    `endif 

    //for zknh
    reg [NUM_LANES-1:0][31:0] inb;
    `ifndef XLEN_64
    reg [NUM_LANES-1:0][`XLEN-1:0] inb_32;
    `endif 


    //wire [NUM_LANES-1:0][`XLEN-1:0] khu_in1_PC  = execute_if.data.op_args.khu.use_PC ? {NUM_LANES{to_fullPC(execute_if.data.PC)}} : khu_in1;
    wire [NUM_LANES-1:0][`XLEN-1:0] khu_in2_imm = execute_if.data.op_args.khu.use_imm ? {NUM_LANES{`SEXT(`XLEN, execute_if.data.op_args.khu.imm)}} : khu_in2;

    for (genvar i = 0; i < NUM_LANES; ++i) begin : g_zbkb_result
        always @(*) begin
            shamt[i] = khu_in2[i][`CLOG2(`XLEN) - 1 : 0];
            shamt_imm[i] = khu_in2_imm[i][`CLOG2(`XLEN) - 1 : 0];
            `ifdef XLEN_64
                shamt_xlen[i] = khu_in2[i][4 : 0];
                shamt_imm_xlen[i] = khu_in2_imm[i][4 : 0];
                khu_in1_xlen[i] = khu_in1[i][31 : 0];
                result_w[i] = 32'b0;
            `endif
            case (khu_op)
                    INST_KHU_ROR: zbkb_result[i] = (khu_in1[i] >> shamt[i]) | (khu_in1[i] << (`XLEN - shamt[i])); // ROR
                    INST_KHU_ROL: zbkb_result[i] = (khu_in1[i] << shamt[i]) | (khu_in1[i] >> (`XLEN - shamt[i]));// ROL
                    INST_KHU_RORI: zbkb_result[i] = (khu_in1[i] >> shamt_imm[i]) | (khu_in1[i] << (`XLEN - shamt_imm[i])); // RORI
                    INST_KHU_PACK: zbkb_result[i] = {khu_in2[i][`XLEN/2 -1 : 0], khu_in1[i][`XLEN/2 -1 : 0]}; //PACK
                    INST_KHU_PACKH: zbkb_result[i] = {{(`XLEN-16){1'b0}}, khu_in2[i][7 : 0], khu_in1[i][7 : 0]}; //PACKH zero extension
                    INST_KHU_BREV8: begin //BREV8
                        for (integer j = 0; j < `XLEN; j += 8) begin
                            zbkb_result[i][j +: 8] = {
                                khu_in1[i][j+0], khu_in1[i][j+1],
                                khu_in1[i][j+2], khu_in1[i][j+3],
                                khu_in1[i][j+4], khu_in1[i][j+5],
                                khu_in1[i][j+6], khu_in1[i][j+7]
                            };
                        end
                    end
                    INST_KHU_REV8: begin //REV8
                        for (integer z = 0; z < `XLEN; z += 8) begin
                            zbkb_result[i][z +: 8] = khu_in1[i][`XLEN - 1 - z -: 8];
                        end   
                    end  
                `ifdef XLEN_64
                    INST_KHU_RORW: begin //RORW
                        result_w[i] = ((khu_in1_xlen[i] >> shamt_xlen[i]) | (khu_in1_xlen[i] << (32 - shamt_xlen[i])));
                        zbkb_result[i] = `SEXT(`XLEN, result_w[i]); // RORW
                    end
                    INST_KHU_ROLW: begin //ROLW
                        result_w[i] = ((khu_in1_xlen[i] << shamt_xlen[i]) | (khu_in1_xlen[i] >> (32 - shamt_xlen[i])));
                        zbkb_result[i] = `SEXT(`XLEN, result_w[i]); // ROLW
                    end
                    INST_KHU_RORIW: begin //RORIW
                        result_w[i] = ((khu_in1_xlen[i] >> shamt_imm_xlen[i]) | (khu_in1_xlen[i] << (32 - shamt_imm_xlen[i])));
                        zbkb_result[i] = `SEXT(`XLEN, result_w[i]); // RORIW
                    end
                    INST_KHU_PACKW: begin //PACKW
                        result_w[i] = {khu_in2[i][15 : 0], khu_in1[i][15 : 0]}; 
                        zbkb_result[i] = `SEXT(`XLEN, result_w[i]); //PACKW
                    end
                `else //Instruction only for RISCV 32
                    INST_KHU_ZIP: begin //ZIP
                        for (integer j = 0; j < `XLEN / 2 - 1; j++) begin
                            zbkb_result[i][2 * j] = khu_in1[i][j];
                            zbkb_result[i][2 * j + 1] = khu_in1[i][j + `XLEN / 2];
                        end
                    end
                    INST_KHU_UNZIP: begin //UNZIP
                        for (integer j = 0; j < `XLEN / 2 - 1; j++) begin 
                            zbkb_result[i][j] = khu_in1[i][2 * j];
                            zbkb_result[i][j + `XLEN / 2] = khu_in1[i][2 * j + 1];
                        end
                    end
                `endif 
                default :  begin
                    zbkb_result[i] = `XLEN'b0;
                end
            endcase 
        end
    end

    for (genvar i = 0; i < NUM_LANES; ++i) begin : g_zbkc_zbkx_result
        always @(*) begin
            result[i]=`XLEN'b0;
            case (khu_op)
                INST_KHU_CLMUL: begin //CLMUL  
                    for (integer j = 0; j < `XLEN; j++) begin
                        if(((khu_in2[i] >> j) & 1) == `XLEN'b1)
                            result[i] = result[i] ^ (khu_in1[i] << j);
                    end 
                zbkc_zbkx_result[i] = result[i];
                end
                INST_KHU_CLMULH: begin //CLMULH
                    for (integer j = 1; j < `XLEN; j++) begin
                        if(((khu_in2[i] >> j) & 1) == `XLEN'b1)
                            result[i] = result[i] ^ (khu_in1[i] >> (`XLEN -j));
                    end 
                zbkc_zbkx_result[i] = result[i];
                end
                INST_KHU_XPERM8: begin //XPERM8
                    for (integer j = 0; j < `XLEN; j += 8) begin
                        if (khu_in2[i][j +: 8] < `XLEN / 8) begin
                            result[i][j +: 8] = khu_in1[i][(khu_in2[i][j +: 8]) * 8 +: 8]; 
                        end
                        else begin
                            result[i][j +: 8] = 8'b0;
                        end
                    end
                    zbkc_zbkx_result[i] = result[i];
                end
                INST_KHU_XPERM4: begin //XPERM4
                    for (integer j = 0; j < `XLEN; j += 4) begin
                            if ({4'b0, khu_in2[i][j +: 4]} < `XLEN / 4) begin
                                result[i][j +: 4] = khu_in1[i][(khu_in2[i][j +: 4]) * 4 +: 4]; 
                            end
                            else begin
                                result[i][j +: 4] = 4'b0;
                            end
                        end
                        zbkc_zbkx_result[i] = result[i];
                    end
                default :  begin
                    zbkc_zbkx_result[i] = `XLEN'b0;
                end
            endcase 
        end
    end

        for (genvar i = 0; i < NUM_LANES; ++i) begin : g_zknd_zkne_result
        always @(*) begin
            `ifndef XLEN_64 //only riscv 32
                result[i]=`XLEN'b0;
                mixed[i] = 32'b0;
                shamt_aes[i] = 5'b0;
                si[i] = 8'b0;
                so_32[i] = 32'b0;
                so_8[i] = 8'b0;
                tmp[i] = 32'b0;
                result_aes[i] = 32'b0;
            `endif 
            `ifdef XLEN_64
                sr[i] = 64'b0;
                sb[i] = 64'b0;
                w0[i] = 32'b0;
                w1[i] = 32'b0;
                tmp1[i] = 32'b0;
                tmp2[i] = 32'b0;
                tmp3[i] = 32'b0;
                rc[i] = 32'b0;
            `endif
            case (khu_op)
                `ifndef XLEN_64 //only riscv 32
                INST_KHU_AES32DSI: begin  //AES32DSI
                    shamt_aes[i] = {bs, 3'b0}; 
                    tmp[i] = khu_in2[i][31:0] >> shamt_aes[i];
                    si[i] = tmp[i][7:0];
                    so_32[i] = {24'b0, aes_sbox_inv(si[i])};
                    result_aes[i] = khu_in1[i][31:0] ^ ((so_32[i][31:0] << shamt_aes[i]) | (so_32[i][31:0] >> (32 - shamt_aes[i])));
                    zknd_zkne_result[i] = `SEXT(`XLEN, result_aes[i]);
                end
                INST_KHU_AES32DSMI: begin //AES32DSMI 
                    shamt_aes[i] = {bs, 3'b0}; 
                    tmp[i] = khu_in2[i][31:0] >> shamt_aes[i];
                    si[i] = tmp[i][7:0];
                    so_8[i] = aes_sbox_inv(si[i]);
                    mixed[i] = aes_mixcolumn_byte_inv(so_8[i]);
                    result_aes[i] = khu_in1[i][31:0] ^ ((mixed[i][31:0] << shamt_aes[i]) | (mixed[i][31:0] >> (32 - shamt_aes[i])));
                    zknd_zkne_result[i] = `SEXT(`XLEN, result_aes[i]);
                end
                INST_KHU_AES32ESI: begin  //AES32ESI
                    shamt_aes[i] = {bs, 3'b0}; 
                    tmp[i] = khu_in2[i][31:0] >> shamt_aes[i];
                    si[i] = tmp[i][7:0];
                    so_32[i] = {24'b0, aes_sbox_fwd(si[i])};
                    result_aes[i] = khu_in1[i][31:0] ^ ((so_32[i][31:0] << shamt_aes[i]) | (so_32[i][31:0] >> (32 - shamt_aes[i])));
                    zknd_zkne_result[i] = `SEXT(`XLEN, result_aes[i]);
                end
                INST_KHU_AES32ESMI: begin //AES32ESMI 
                    shamt_aes[i] = {bs, 3'b0}; 
                    tmp[i] = khu_in2[i][31:0] >> shamt_aes[i];
                    si[i] = tmp[i][7:0];
                    so_8[i] = aes_sbox_fwd(si[i]);
                    mixed[i] = aes_mixcolumn_byte_fwd(so_8[i]);
                    result_aes[i] = khu_in1[i][31:0] ^ ((mixed[i][31:0] << shamt_aes[i]) | (mixed[i][31:0] >> (32 - shamt_aes[i])));
                    zknd_zkne_result[i] = `SEXT(`XLEN, result_aes[i]);
                end
                `endif
                `ifdef XLEN_64
                INST_KHU_AES64DS: begin //AES64DS
                    sr[i] = aes_rv64_shiftrows_inv(khu_in2[i], khu_in1[i]);
                    zknd_zkne_result[i] = aes_apply_inv_sbox_to_each_byte(sr[i]);
                end
                INST_KHU_AES64DSM: begin //AES64DSM
                    sr[i] = aes_rv64_shiftrows_inv(khu_in2[i], khu_in1[i]);
                    sb[i] = aes_apply_inv_sbox_to_each_byte(sr[i]); 
                    zknd_zkne_result[i] = {aes_mixcolumn_inv(sb[i][63:32]), aes_mixcolumn_inv(sb[i][31:0])};
                end
                INST_KHU_AES64IM: begin //AES64IM
                    w0[i] = aes_mixcolumn_inv(khu_in1[i][31:0]);
                    w1[i] = aes_mixcolumn_inv(khu_in1[i][63:32]);
                    zknd_zkne_result[i] = {w1[i], w0[i]};
                end
                INST_KHU_AES64KS1I: begin //AES64KS1I
                    if (khu_in2[i][3:0] > 10) begin
                        //handle_illegal();
                        zknd_zkne_result[i] = `XLEN'b0;
                    end
                    else begin
                            tmp1[i] = khu_in1[i][63:32];
                            rc[i] = aes_decode_rcon(khu_in2[i][3:0]);
                            if(khu_in2[i][3:0] == 4'hA) begin
                                tmp2[i] = tmp1[i];
                            end
                            else begin
                                tmp2[i] = (tmp1[i] >> 8) | (tmp1[i] << (24)); 
                            end
                            tmp3[i] = aes_subword_fwd(tmp2[i]);
                            zknd_zkne_result[i] = {{(`XLEN-64){1'b0}}, (tmp3[i] ^ rc[i]), (tmp3[i] ^ rc[i])}; //zero extension
                    end     
                end
                INST_KHU_AES64KS2: begin //AES64KS2
                    w0[i] = khu_in1[i][63:32] ^ khu_in2[i][31:0];
                    w1[i] = khu_in1[i][63:32] ^ khu_in2[i][31:0] ^ khu_in2[i][63:32];
                    zknd_zkne_result[i] = {w1[i], w0[i]};
                end
                INST_KHU_AES64ES: begin //AES64ES
                    sr[i] = aes_rv64_shiftrows_fwd(khu_in2[i], khu_in1[i]);
                    zknd_zkne_result[i] = aes_apply_fwd_sbox_to_each_byte(sr[i]);
                end
                INST_KHU_AES64ESM: begin //AES64ESM
                    sr[i] = aes_rv64_shiftrows_fwd(khu_in2[i], khu_in1[i]);
                    sb[i] = aes_apply_fwd_sbox_to_each_byte(sr[i]); 
                    zknd_zkne_result[i] = {aes_mixcolumn_fwd(sb[i][63:32]), aes_mixcolumn_fwd(sb[i][31:0])};
                end
                `endif 
                 default :  begin
                    zknd_zkne_result[i] = `XLEN'b0;
                end
            endcase 
        end
    end

    for (genvar i = 0; i < NUM_LANES; ++i) begin : g_zknh_result
        always @(*) begin
            inb[i] = 32'b0;
            `ifndef XLEN_64
                inb_32[i] = `XLEN'b0;
            `endif 
            case (khu_op)
                INST_KHU_SHA256SIG0: begin
                    inb[i] = ((khu_in1[i][31:0] >> 7) | (khu_in1[i][31:0] << 25)) ^ ((khu_in1[i][31:0] >> 18) | (khu_in1[i][31:0] << 14)) ^ (khu_in1[i][31:0] >> 3);
                    zknh_result[i] = `SEXT(`XLEN, inb[i]);
                end
                INST_KHU_SHA256SIG1: begin
                    inb[i] = ((khu_in1[i][31:0] >> 17) | (khu_in1[i][31:0] << 15)) ^ ((khu_in1[i][31:0] >> 19) | (khu_in1[i][31:0] << 13)) ^ (khu_in1[i][31:0] >> 10);
                    zknh_result[i] = `SEXT(`XLEN, inb[i]);
                end
                INST_KHU_SHA256SUM0: begin
                    inb[i] = ((khu_in1[i][31:0] >> 2) | (khu_in1[i][31:0] << 30)) ^ ((khu_in1[i][31:0] >> 13) | (khu_in1[i][31:0] << 19)) ^ ((khu_in1[i][31:0] >> 22) | (khu_in1[i][31:0] << 10));
                    zknh_result[i] = `SEXT(`XLEN, inb[i]);
                end
                INST_KHU_SHA256SUM1: begin
                    inb[i] = ((khu_in1[i][31:0] >> 6) | (khu_in1[i][31:0] << 26)) ^ ((khu_in1[i][31:0] >> 11) | (khu_in1[i][31:0] << 21)) ^ ((khu_in1[i][31:0] >> 25) | (khu_in1[i][31:0] << 7));
                    zknh_result[i] = `SEXT(`XLEN, inb[i]);
                end
                `ifndef XLEN_64 //only riscv 32
                    INST_KHU_SHA512SIG0H: begin
                        inb_32[i] = (khu_in1[i] >> 1) ^ (khu_in1[i] >> 7) ^ (khu_in1[i] >> 8) ^ (khu_in2[i] << 31) ^ (khu_in2[i] << 24);
                        zknh_result[i] = `SEXT(`XLEN, inb_32[i]);
                    end
                    INST_KHU_SHA512SIG0L: begin
                        inb_32[i] = (khu_in1[i] >> 1) ^ (khu_in1[i] >> 7) ^ (khu_in1[i] >> 8) ^ (khu_in2[i] << 31) ^ (khu_in2[i] << 25) ^ (khu_in2[i] << 24);
                        zknh_result[i] = `SEXT(`XLEN, inb_32[i]);
                    end
                    INST_KHU_SHA512SIG1H: begin
                        inb_32[i] = (khu_in1[i] << 3) ^ (khu_in1[i] >> 6) ^ (khu_in1[i] >> 19) ^ (khu_in2[i] >> 29) ^ (khu_in2[i] << 13);
                        zknh_result[i] = `SEXT(`XLEN, inb_32[i]);
                    end
                    INST_KHU_SHA512SIG1L: begin
                        inb_32[i] = (khu_in1[i] << 3) ^ (khu_in1[i] >> 6) ^ (khu_in1[i] >> 19) ^ (khu_in2[i] >> 29) ^ (khu_in2[i] << 26) ^ (khu_in2[i] << 13);
                        zknh_result[i] = `SEXT(`XLEN, inb_32[i]);
                    end
                    INST_KHU_SHA512SIG0R: begin
                        inb_32[i] = (khu_in1[i] << 25) ^ (khu_in1[i] << 30) ^ (khu_in1[i] >> 28) ^ (khu_in2[i] >> 7) ^ (khu_in2[i] >> 2) ^ (khu_in2[i] << 4);
                        zknh_result[i] = `SEXT(`XLEN, inb_32[i]);
                    end
                    INST_KHU_SHA512SIG1R: begin
                        inb_32[i] = (khu_in1[i] << 23) ^ (khu_in1[i] >> 14) ^ (khu_in1[i] >> 18) ^ (khu_in2[i] >> 9) ^ (khu_in2[i] << 18) ^ (khu_in2[i] << 14);
                        zknh_result[i] = `SEXT(`XLEN, inb_32[i]);
                    end
                `endif 
                `ifdef XLEN_64 //only riscv 64
                INST_KHU_SHA512SIG0: begin
                    zknh_result[i]  = ((khu_in1[i] >> 1) | (khu_in1[i] << 63)) ^ ((khu_in1[i] >> 8) | (khu_in1[i] << 56)) ^ (khu_in1[i] >> 7);
                end
                INST_KHU_SHA512SIG1: begin
                    zknh_result[i]  = ((khu_in1[i] >> 19) | (khu_in1[i] << 45)) ^ ((khu_in1[i] >> 61) | (khu_in1[i] << 3)) ^ (khu_in1[i] >> 6);
                end
                INST_KHU_SHA512SUM0: begin
                    zknh_result[i]  = ((khu_in1[i] >> 28) | (khu_in1[i] << 36)) ^ ((khu_in1[i] >> 34) | (khu_in1[i] << 30)) ^ ((khu_in1[i] >> 39) | (khu_in1[i] << 25));
                end
                INST_KHU_SHA512SUM1: begin
                    zknh_result[i]  = ((khu_in1[i] >> 14) | (khu_in1[i] << 50)) ^ ((khu_in1[i] >> 18) | (khu_in1[i] << 46)) ^ ((khu_in1[i] >> 41) | (khu_in1[i] << 23));
                end
                `endif 
                default :  begin
                    zknh_result[i] = `XLEN'b0;
                end
            endcase
        end
    end

    for (genvar i = 0; i < NUM_LANES; ++i) begin : g_khu_result
       always @(*) begin
            khu_result[i] = `XLEN'b0; 
            if (execute_if.data.op_args.khu.xtype == KHU_TYPE_ZBKB) begin
                khu_result[i] = zbkb_result[i];       
            end
            else if(execute_if.data.op_args.khu.xtype == KHU_TYPE_ZBKC_ZBKX) begin
                khu_result[i] = zbkc_zbkx_result[i];
            end
            else if(execute_if.data.op_args.khu.xtype == KHU_TYPE_ZKND_ZKNE) begin
                khu_result[i] = zknd_zkne_result[i];
            end
            else if(execute_if.data.op_args.khu.xtype == KHU_TYPE_ZKNH) begin
                khu_result[i] = zknh_result[i];
            end
        end
    end

    wire [PC_BITS-1:0] PC_r;

    VX_elastic_buffer #(
        .DATAW (UUID_WIDTH + NW_WIDTH + NUM_LANES + NUM_REGS_BITS + 1 + PID_WIDTH + 1 + 1 + (NUM_LANES * `XLEN) + PC_BITS)
    ) rsp_buf (
        .clk      (clk),
        .reset    (reset),
        .valid_in (execute_if.valid),
        .ready_in (execute_if.ready),
        .data_in  ({execute_if.data.uuid, execute_if.data.wid, execute_if.data.tmask, execute_if.data.rd, execute_if.data.wb, execute_if.data.pid, execute_if.data.sop, execute_if.data.eop, khu_result,   execute_if.data.PC}),
        .data_out ({result_if.data.uuid,  result_if.data.wid,  result_if.data.tmask,  result_if.data.rd,  result_if.data.wb,  result_if.data.pid,  result_if.data.sop,  result_if.data.eop,  khu_result_r, PC_r}),
        .valid_out (result_if.valid),
        .ready_out (result_if.ready)
    );
    
    for (genvar i = 0; i < NUM_LANES; ++i) begin : g_result
        assign result_if.data.data[i] = khu_result_r[i];
    end

    assign result_if.data.PC = PC_r;

endmodule
