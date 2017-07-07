import math
import numpy as np


class vmodule:
  def __init__(self,name):
    self.name = name
    self.params = []
    self.ports = []

  def add_param(self,name,init=None):
    self.params.append([name,init])

  def add_input(self,name, width):
    self.ports.append(["input",name,width])
  
  def add_output(self,name, width):
    self.ports.append(["output",name,width])
  
  def add_body(self,body):
    self.body = body

  def to_string(self):
    assert len(self.ports) > 0
    assert self.body
    s = "module %s " % self.name
    if (len(self.params) > 0):
      ps = []
      for pa in self.params:
        p = "parameter " + pa[0]
        if (not pa[1] is None):
          p += "="+str(pa[1])
        ps.append(p)
      s += "#(%s) " % ",".join(ps)
    ports = []
    for port in self.ports:
      p = "  " + port[0]
      width = port[2]
      if (width != 1):
        if type(width) is str:
          p += " [%s-1:0]" % width
        elif type(width) is int:
          p += "[%s:0]" % str(width-1)
        else:
          assert(False)
      p += " " + port[1]
      ports.append(p)
    ports.sort()
    s += "(\n%s\n);\n" % ",\n".join(ports)
    s += self.body + "\n"
    s += "endmodule\n\n"
    return s

if __name__ == "__main__":
  
  
  
  ops = {
    "unary":{
      "not":"~in",
      "neg":"-in"
    },
    "unaryReduce":{
      "andr":"&in",
      "orr":"|in",
      "xorr":"^in"
    },
    "binary":{
      "and":"in0 & in1",
      "or":"in0 | in1",
      "xor":"in0 ^ in1",
      "dshl":"in0 << in1",
      "dlshr":"in0 >> in1",
      "dashr":"$signed(in0) >>> in1", #Could be buggy
      "add":"in0 + in1",
      "sub":"in0 - in1",
      #"mul":"in0 * in1",
      #"udiv":"in0 / in1",
      #"urem":"in0 % in1",
      #"sdiv":"$signed(in0) / $signed(in1)", #Could be buggy
      #"srem":"$signed(in0) % $signed(in1)", 
      #"smod":"$signed(in0) % $signed(in1)", #TODO definitely wrong
    },
    "binaryReduce":{
      "eq":"in0 == in1",
      "slt":"$signed(in0) < $signed(in1)",
      "sgt":"$signed(in0) > $signed(in1)",
      "sle":"$signed(in0) <= $signed(in1)",
      "sge":"$signed(in0) >= $signed(in1)",
      "ult":"in0 < in1",
      "ugt":"in0 > in1",
      "ult":"in0 <= in1",
      "uge":"in0 >= in1"
    }
  }

  with open("stdlib.v","w") as f:
    
    f.write("//unary ops\n")
    for op, exp in ops["unary"].iteritems():
      v = vmodule(op.capitalize())
      v.add_param("WIDTH",16)
      v.add_input("in","WIDTH")
      v.add_output("out","WIDTH")
      v.add_body("  assign out = %s;" % exp)
      f.write(v.to_string())
    
    f.write("//unaryReduce ops\n")
    for op, exp in ops["unaryReduce"].iteritems():
      v = vmodule(op.capitalize())
      v.add_param("WIDTH",16)
      v.add_input("in","WIDTH")
      v.add_output("out",1)
      v.add_body("  assign out = %s;" % exp)
      f.write(v.to_string())
    
    f.write("//binary ops\n")
    for op, exp in ops["binary"].iteritems():
      v = vmodule(op.capitalize())
      v.add_param("WIDTH",16)
      v.add_input("in0","WIDTH")
      v.add_input("in1","WIDTH")
      v.add_output("out","WIDTH")
      v.add_body("  assign out = %s;" % exp)
      f.write(v.to_string())
    
    f.write("//binaryReduce ops\n")
    for op, exp in ops["binaryReduce"].iteritems():
      v = vmodule(op.capitalize())
      v.add_param("WIDTH",16)
      v.add_input("in","WIDTH")
      v.add_output("out",1)
      v.add_body("  assign out = %s;" % exp)
      f.write(v.to_string())
    
    #Do the mux
    f.write("//ternary op\n")
    v = vmodule("Mux")
    v.add_param("WIDTH",16)
    v.add_input("d0","WIDTH")
    v.add_input("d1","WIDTH")
    v.add_input("sel",1)
    v.add_output("out","WIDTH")
    v.add_body("  assign out = sel ? d0 : d1;")
    f.write(v.to_string())

    #Now do the registers
    #name is the regex: Reg_(P|N)R?C?E?
    #P=posedge clock,
    #N=negedge clock,
    #R=rst (asynchronous)
    #C=clr (sychronous)
    #E=en

    f.write("//Now all the register permutations\n")
    #posedge = bit 0
    #rst = bit 1
    #clr = bit 2
    #en = bit 3
    def rexpr(clr,en):
      expr = "D"
      pen = "D"
      if (clr):
        pen = "(clr ? INIT : D)"
      if (en):
        expr = "en ? %s : r" % pen
      return expr
    for i in range(16):
      posedge = (i>>0) & 1
      rst = (i>>1) & 1
      clr = (i>>2) & 1
      en = (i>>3) & 1
      if (rst and clr):
        continue
      body = "  reg [WIDTH-1:0] r;\n"
      body += "  always @(%s clk" % ("posedge" if posedge else "negedge")
      if (rst):
        body += ", negedge rst"
      body += ") begin\n"
      if (rst):
        body += "    if (!rst) r <= INIT;\n"
        body += "    else r <= %s;\n" % rexpr(clr,en)
      else:
        body += "    r <= %s;\n" % rexpr(clr,en)
      body += "  end\n"
      body += "  assign Q = r;"
      name = "Reg_" + ("P" if posedge else "N")
      if (rst):
        name += "R"
      if (clr):
        name += "C"
      if (en):
        name += "E"
      v = vmodule(name)
      v.add_param("WIDTH",16)
      if (rst or clr):
        v.add_param("INIT",0)
      v.add_input("D","WIDTH")
      v.add_input("clk",1)
      v.add_output("Q","WIDTH")
      if (rst):
        v.add_input("rst",1)
      if (clr):
        v.add_input("clr",1)
      if (en):
        v.add_input("en",1)

      v.add_body(body)
      f.write(v.to_string())
      



