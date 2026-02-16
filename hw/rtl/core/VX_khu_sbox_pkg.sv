`ifndef VX_KHU_SBOX_PKG_SV
`define VX_KHU_SBOX_PKG_SV

package VX_khu_sbox_pkg;

// =========================
// AES forward S-Box

localparam logic [7:0] aes_sbox_fwd_table [0:255] = '{
    8'h63, 8'h7c, 8'h77, 8'h7b, 8'hf2, 8'h6b, 8'h6f, 8'hc5, 8'h30, 8'h01, 8'h67, 8'h2b, 8'hfe,
    8'hd7, 8'hab, 8'h76, 8'hca, 8'h82, 8'hc9, 8'h7d, 8'hfa, 8'h59, 8'h47, 8'hf0, 8'had, 8'hd4,
    8'ha2, 8'haf, 8'h9c, 8'ha4, 8'h72, 8'hc0, 8'hb7, 8'hfd, 8'h93, 8'h26, 8'h36, 8'h3f, 8'hf7,
    8'hcc, 8'h34, 8'ha5, 8'he5, 8'hf1, 8'h71, 8'hd8, 8'h31, 8'h15, 8'h04, 8'hc7, 8'h23, 8'hc3,
    8'h18, 8'h96, 8'h05, 8'h9a, 8'h07, 8'h12, 8'h80, 8'he2, 8'heb, 8'h27, 8'hb2, 8'h75, 8'h09,
    8'h83, 8'h2c, 8'h1a, 8'h1b, 8'h6e, 8'h5a, 8'ha0, 8'h52, 8'h3b, 8'hd6, 8'hb3, 8'h29, 8'he3,
    8'h2f, 8'h84, 8'h53, 8'hd1, 8'h00, 8'hed, 8'h20, 8'hfc, 8'hb1, 8'h5b, 8'h6a, 8'hcb, 8'hbe,
    8'h39, 8'h4a, 8'h4c, 8'h58, 8'hcf, 8'hd0, 8'hef, 8'haa, 8'hfb, 8'h43, 8'h4d, 8'h33, 8'h85,
    8'h45, 8'hf9, 8'h02, 8'h7f, 8'h50, 8'h3c, 8'h9f, 8'ha8, 8'h51, 8'ha3, 8'h40, 8'h8f, 8'h92,
    8'h9d, 8'h38, 8'hf5, 8'hbc, 8'hb6, 8'hda, 8'h21, 8'h10, 8'hff, 8'hf3, 8'hd2, 8'hcd, 8'h0c,
    8'h13, 8'hec, 8'h5f, 8'h97, 8'h44, 8'h17, 8'hc4, 8'ha7, 8'h7e, 8'h3d, 8'h64, 8'h5d, 8'h19,
    8'h73, 8'h60, 8'h81, 8'h4f, 8'hdc, 8'h22, 8'h2a, 8'h90, 8'h88, 8'h46, 8'hee, 8'hb8, 8'h14,
    8'hde, 8'h5e, 8'h0b, 8'hdb, 8'he0, 8'h32, 8'h3a, 8'h0a, 8'h49, 8'h06, 8'h24, 8'h5c, 8'hc2,
    8'hd3, 8'hac, 8'h62, 8'h91, 8'h95, 8'he4, 8'h79, 8'he7, 8'hc8, 8'h37, 8'h6d, 8'h8d, 8'hd5,
    8'h4e, 8'ha9, 8'h6c, 8'h56, 8'hf4, 8'hea, 8'h65, 8'h7a, 8'hae, 8'h08, 8'hba, 8'h78, 8'h25,
    8'h2e, 8'h1c, 8'ha6, 8'hb4, 8'hc6, 8'he8, 8'hdd, 8'h74, 8'h1f, 8'h4b, 8'hbd, 8'h8b, 8'h8a,
    8'h70, 8'h3e, 8'hb5, 8'h66, 8'h48, 8'h03, 8'hf6, 8'h0e, 8'h61, 8'h35, 8'h57, 8'hb9, 8'h86,
    8'hc1, 8'h1d, 8'h9e, 8'he1, 8'hf8, 8'h98, 8'h11, 8'h69, 8'hd9, 8'h8e, 8'h94, 8'h9b, 8'h1e,
    8'h87, 8'he9, 8'hce, 8'h55, 8'h28, 8'hdf, 8'h8c, 8'ha1, 8'h89, 8'h0d, 8'hbf, 8'he6, 8'h42,
    8'h68, 8'h41, 8'h99, 8'h2d, 8'h0f, 8'hb0, 8'h54, 8'hbb, 8'h16
};

// =========================
// AES inverse S-Box

localparam logic [7:0] aes_sbox_inv_table [0:255] = '{
    8'h52, 8'h09, 8'h6a, 8'hd5, 8'h30, 8'h36, 8'ha5, 8'h38, 8'hbf, 8'h40, 8'ha3, 8'h9e, 8'h81,
    8'hf3, 8'hd7, 8'hfb, 8'h7c, 8'he3, 8'h39, 8'h82, 8'h9b, 8'h2f, 8'hff, 8'h87, 8'h34, 8'h8e,
    8'h43, 8'h44, 8'hc4, 8'hde, 8'he9, 8'hcb, 8'h54, 8'h7b, 8'h94, 8'h32, 8'ha6, 8'hc2, 8'h23,
    8'h3d, 8'hee, 8'h4c, 8'h95, 8'h0b, 8'h42, 8'hfa, 8'hc3, 8'h4e, 8'h08, 8'h2e, 8'ha1, 8'h66,
    8'h28, 8'hd9, 8'h24, 8'hb2, 8'h76, 8'h5b, 8'ha2, 8'h49, 8'h6d, 8'h8b, 8'hd1, 8'h25, 8'h72,
    8'hf8, 8'hf6, 8'h64, 8'h86, 8'h68, 8'h98, 8'h16, 8'hd4, 8'ha4, 8'h5c, 8'hcc, 8'h5d, 8'h65,
    8'hb6, 8'h92, 8'h6c, 8'h70, 8'h48, 8'h50, 8'hfd, 8'hed, 8'hb9, 8'hda, 8'h5e, 8'h15, 8'h46,
    8'h57, 8'ha7, 8'h8d, 8'h9d, 8'h84, 8'h90, 8'hd8, 8'hab, 8'h00, 8'h8c, 8'hbc, 8'hd3, 8'h0a,
    8'hf7, 8'he4, 8'h58, 8'h05, 8'hb8, 8'hb3, 8'h45, 8'h06, 8'hd0, 8'h2c, 8'h1e, 8'h8f, 8'hca,
    8'h3f, 8'h0f, 8'h02, 8'hc1, 8'haf, 8'hbd, 8'h03, 8'h01, 8'h13, 8'h8a, 8'h6b, 8'h3a, 8'h91,
    8'h11, 8'h41, 8'h4f, 8'h67, 8'hdc, 8'hea, 8'h97, 8'hf2, 8'hcf, 8'hce, 8'hf0, 8'hb4, 8'he6,
    8'h73, 8'h96, 8'hac, 8'h74, 8'h22, 8'he7, 8'had, 8'h35, 8'h85, 8'he2, 8'hf9, 8'h37, 8'he8,
    8'h1c, 8'h75, 8'hdf, 8'h6e, 8'h47, 8'hf1, 8'h1a, 8'h71, 8'h1d, 8'h29, 8'hc5, 8'h89, 8'h6f,
    8'hb7, 8'h62, 8'h0e, 8'haa, 8'h18, 8'hbe, 8'h1b, 8'hfc, 8'h56, 8'h3e, 8'h4b, 8'hc6, 8'hd2,
    8'h79, 8'h20, 8'h9a, 8'hdb, 8'hc0, 8'hfe, 8'h78, 8'hcd, 8'h5a, 8'hf4, 8'h1f, 8'hdd, 8'ha8,
    8'h33, 8'h88, 8'h07, 8'hc7, 8'h31, 8'hb1, 8'h12, 8'h10, 8'h59, 8'h27, 8'h80, 8'hec, 8'h5f,
    8'h60, 8'h51, 8'h7f, 8'ha9, 8'h19, 8'hb5, 8'h4a, 8'h0d, 8'h2d, 8'he5, 8'h7a, 8'h9f, 8'h93,
    8'hc9, 8'h9c, 8'hef, 8'ha0, 8'he0, 8'h3b, 8'h4d, 8'hae, 8'h2a, 8'hf5, 8'hb0, 8'hc8, 8'heb,
    8'hbb, 8'h3c, 8'h83, 8'h53, 8'h99, 8'h61, 8'h17, 8'h2b, 8'h04, 8'h7e, 8'hba, 8'h77, 8'hd6,
    8'h26, 8'he1, 8'h69, 8'h14, 8'h63, 8'h55, 8'h21, 8'h0c, 8'h7d

};

// =========================
// Functions

function automatic logic bit_to_bool(input logic x);
    bit_to_bool = x ? 1'b1 : 1'b0;
endfunction

function automatic logic [7:0] xt2(input logic [7:0] x);
    xt2 = ((x << 1) ^ (bit_to_bool(x[7]) ? 8'h1b : 8'h00));
endfunction

function automatic logic [7:0] getbyte(input logic [63:0] x, input integer i);
    getbyte = x[i*8 +: 8];   
endfunction

function automatic logic [7:0] gfmul(input logic [7:0] x, input logic[3:0] y);
    gfmul = ((bit_to_bool(y[0]) ? x : 8'h00) ^ (bit_to_bool(y[1]) ? xt2(x) : 8'h00) ^ (bit_to_bool(y[2]) ? xt2(xt2(x)) : 8'h00) ^ (bit_to_bool(y[3]) ? xt2(xt2(xt2(x))) : 8'h00));
endfunction

function automatic logic [7:0] aes_sbox_inv(input logic [7:0] x);
    aes_sbox_inv = aes_sbox_inv_table[x];
endfunction

function automatic logic [31:0] aes_mixcolumn_byte_inv(input logic [7:0] so);
    aes_mixcolumn_byte_inv = {gfmul(so, 4'hb), gfmul(so, 4'hd), gfmul(so, 4'h9), gfmul(so, 4'he)};
endfunction

function automatic logic [63:0] aes_apply_inv_sbox_to_each_byte(input logic [63:0] x);
    aes_apply_inv_sbox_to_each_byte = {aes_sbox_inv(x[63:56]), aes_sbox_inv(x[55:48]), aes_sbox_inv(x[47:40]), aes_sbox_inv(x[39:32]),
                                        aes_sbox_inv(x[31:24]), aes_sbox_inv(x[23:16]), aes_sbox_inv(x[15:8]), aes_sbox_inv(x[7:0])};
endfunction

function automatic logic [63:0] aes_rv64_shiftrows_inv(input logic [63:0] rs2, input logic [63:0] rs1);
    aes_rv64_shiftrows_inv = {getbyte(rs2, 3), getbyte(rs2, 6), getbyte(rs1, 1), getbyte(rs1, 4), getbyte(rs1, 7), getbyte(rs2, 2), getbyte(rs2, 5), getbyte(rs1, 0)};
endfunction

function automatic logic [31:0] aes_mixcolumn_inv(input logic [31:0] x);
    logic [7:0] s0;
    logic [7:0] s1;
    logic [7:0] s2;
    logic [7:0] s3;
    logic [7:0] b0;
    logic [7:0] b1;
    logic [7:0] b2;
    logic [7:0] b3;
    begin
        s0 = x[7:0];
        s1 = x[15:8];
        s2 = x[23:16];
        s3 = x[31:24];
        b0 = gfmul(s0, 4'hE) ^ gfmul(s1, 4'hB) ^ gfmul(s2, 4'hD) ^ gfmul(s3, 4'h9);
        b1 = gfmul(s0, 4'h9) ^ gfmul(s1, 4'hE) ^ gfmul(s2, 4'hB) ^ gfmul(s3, 4'hD);
        b2 = gfmul(s0, 4'hD) ^ gfmul(s1, 4'h9) ^ gfmul(s2, 4'hE) ^ gfmul(s3, 4'hB);
        b3 = gfmul(s0, 4'hB) ^ gfmul(s1, 4'hD) ^ gfmul(s2, 4'h9) ^ gfmul(s3, 4'hE);
        aes_mixcolumn_inv = {b3, b2, b1, b0};
    end
endfunction

function automatic logic [31:0] aes_decode_rcon(input logic [3:0] r);
    case(r)
        4'h0: aes_decode_rcon = 32'h00000001;
        4'h1: aes_decode_rcon = 32'h00000002;
        4'h2: aes_decode_rcon = 32'h00000004;
        4'h3: aes_decode_rcon = 32'h00000008;
        4'h4: aes_decode_rcon = 32'h00000010;
        4'h5: aes_decode_rcon = 32'h00000020;
        4'h6: aes_decode_rcon = 32'h00000040;
        4'h7: aes_decode_rcon = 32'h00000080;
        4'h8: aes_decode_rcon = 32'h0000001b;
        4'h9: aes_decode_rcon = 32'h00000036;
        4'hA: aes_decode_rcon = 32'h00000000;
        4'hB: aes_decode_rcon = 32'h00000000;
        4'hC: aes_decode_rcon = 32'h00000000;
        4'hD: aes_decode_rcon = 32'h00000000;
        4'hE: aes_decode_rcon = 32'h00000000;
        4'hF: aes_decode_rcon = 32'h00000000;
    endcase
endfunction

function automatic logic [7:0] aes_sbox_fwd(input logic [7:0] x);
    aes_sbox_fwd = aes_sbox_fwd_table[x];
endfunction

function automatic logic [31:0] aes_subword_fwd(input logic [31:0] x);
    aes_subword_fwd = {aes_sbox_fwd(x[31:24]), aes_sbox_fwd(x[23:16]), aes_sbox_fwd(x[15:8]), aes_sbox_fwd(x[7:0])};
endfunction

function automatic logic [31:0] aes_mixcolumn_byte_fwd(input logic [7:0] so);
    aes_mixcolumn_byte_fwd = {gfmul(so, 4'h3), so, so, gfmul(so, 4'h2)};
endfunction

function automatic logic [63:0] aes_rv64_shiftrows_fwd(input logic [63:0] rs2, input logic [63:0] rs1);
    aes_rv64_shiftrows_fwd = {getbyte(rs1, 3), getbyte(rs2, 6), getbyte(rs2, 1), getbyte(rs1, 4), getbyte(rs2, 7), getbyte(rs2, 2), getbyte(rs1, 5), getbyte(rs1, 0)};
endfunction

function automatic logic [63:0] aes_apply_fwd_sbox_to_each_byte(input logic [63:0] x);
    aes_apply_fwd_sbox_to_each_byte = {aes_sbox_fwd(x[63:56]), aes_sbox_fwd(x[55:48]), aes_sbox_fwd(x[47:40]), aes_sbox_fwd(x[39:32]),
                                        aes_sbox_fwd(x[31:24]), aes_sbox_fwd(x[23:16]), aes_sbox_fwd(x[15:8]), aes_sbox_fwd(x[7:0])};
endfunction

function automatic logic [7:0] xt3(input logic [7:0] x);
    xt3 = x ^ xt2(x);
endfunction

function automatic logic [31:0] aes_mixcolumn_fwd(input logic [31:0] x);
    logic [7:0] s0;
    logic [7:0] s1;
    logic [7:0] s2;
    logic [7:0] s3;
    logic [7:0] b0;
    logic [7:0] b1;
    logic [7:0] b2;
    logic [7:0] b3;
    begin
        s0 = x[7:0];
        s1 = x[15:8];
        s2 = x[23:16];
        s3 = x[31:24];
        b0 = xt2(s0) ^ xt3(s1) ^ s2 ^ s3;
        b1 = s0 ^ xt2(s1) ^ xt3(s2) ^ s3;
        b2 = s0 ^ s1 ^ xt2(s2) ^ xt3(s3);
        b3 = xt3(s0) ^ s1 ^ s2 ^ xt2(s3);
        aes_mixcolumn_fwd = {b3, b2, b1, b0};
    end
endfunction

endpackage

`endif
