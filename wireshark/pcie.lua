
local padding_proto = Proto("padding", "padding between TLPs")
local p = padding_proto.fields
p.padding_data = ProtoField.new("Padding data", "padding.data", ftypes.BYTES, nil, base.NONE)
p.padding_length = ProtoField.new("Padding length", "padding.len", ftypes.INT32, nil, base.NONE)

local pcie_proto = Proto("PCIe", "PCI Express Transaction Layer Packet(s)")

local f = pcie_proto.fields

-- PCI Express TLP 3DW Header:
-- |       0       |       1       |       2       |       3       |
-- +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
-- | FMT |   Type  |R| TC  |   R   |T|E|Atr| R |       Length      |
-- +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
-- |           Request ID          |      Tag      |LastBE |FirstBE|
-- +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
-- |                           Address                         | R |
-- +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
--

f.tlp_fmt     = ProtoField.new("Packet Format", "pcie.tlp.format", ftypes.UINT8, nil, base.HEX)
local TLPPacketFormat = {
	[0] = "3DW_NO_DATA",
	[1] = "4DW_NO_DATA",
	[2] = "3DW_DATA",
	[3] = "4DW_DATA",
  [4] = "TLP Prefix"
}
f.tlp_fmt = ProtoField.uint8("pcie.tlp.format", "Packet Format", base.DEC, TLPPacketFormat)

f.tlp_type     = ProtoField.new("Packet Format", "pcie.tlp.type", ftypes.UINT8, nil, base.HEX)
local TLPPacketType = {
	[ 0] = "MEMORY",
	[ 4] = "Cfg Type 0",
	[ 5] = "Cfg Type 1",
	[10] = "Cpl"
}
f.tlp_type = ProtoField.uint8("pcie.tlp.pkttype", "Packet Type", base.DEC, TLPPacketType)


f.tlp_tlpType = ProtoField.new("TLP Type", "pcie.tlp.tlpType", ftypes.UINT8, nil, base.HEX)

local tlpTypeLong = {}
tlpTypeLong["MRd"] = "Memory Read Request"
tlpTypeLong["MRdLk"] = "Memory Read Request-Locked"
tlpTypeLong["MWr"] = "Memory Write Request"
tlpTypeLong["IORd"] = "IO Read Request"
tlpTypeLong["IOWr"] = "IO Write Request"
tlpTypeLong["CfgRd0"] = "Configuration Read Type 0"
tlpTypeLong["CfgWr0"] = "Configuration Write Type 0"
tlpTypeLong["CfgRd1"] = "Configuration Read Type 1"
tlpTypeLong["CfgWr1"] = "Configuration Write Type 1"
tlpTypeLong["TCfgRd"] = "Deprecated TLP Type - (Trusted Configuration Read)"
tlpTypeLong["TCfgWr"] = "Deprecated TLP Type - (Trusted Configuration Write)"
tlpTypeLong["Msg"] = "Message Request"
tlpTypeLong["MsgD"] = "Message Request with Data"
tlpTypeLong["Cpl"] = "Completion without Data"
tlpTypeLong["CplD"] = "Completion with Data"
tlpTypeLong["CplLk"] = "Completion for locked Memory Read without Data"
tlpTypeLong["CplDLk"] = "Completion for locked Memory Read with Data"
tlpTypeLong["FetchAdd"] = "Fetch and add AtomicOp Request"
tlpTypeLong["swap"] = "Unconditional swqp AtomicOp Request"
tlpTypeLong["CAS"] = "Compare and Swap AtomicOp Request"
tlpTypeLong["LPrfx"] = "Local TLP Prefix"
tlpTypeLong["EPrfx"] = "End to End TLP Prefix"

local tlpType = {
  [0x00] = "MRd"      ,
  [0x20] = "MRd"      ,
  [0x01] = "MRdLk"    ,
  [0x21] = "MRdLk"    ,
  [0x40] = "MWr"      ,
  [0x60] = "MWr"      ,
  [0x02] = "IORd"     ,
  [0x42] = "IOWr"     ,
  [0x04] = "CfgRd0"   ,
  [0x44] = "CfgWr0"   ,
  [0x05] = "CfgRd1"   ,
  [0x45] = "CfgWr1"   ,
  [0x1B] = "TCfgRd"   ,
  [0x5B] = "TCfgWr"   ,
  [0x30] = "Msg"      ,
  [0x31] = "Msg"      ,
  [0x32] = "Msg"      ,
  [0x33] = "Msg"      ,
  [0x34] = "Msg"      ,
  [0x35] = "Msg"      ,
  [0x36] = "Msg"      ,
  [0x37] = "Msg"      ,
  [0x70] = "MsgD"     ,
  [0x71] = "MsgD"     ,
  [0x72] = "MsgD"     ,
  [0x73] = "MsgD"     ,
  [0x74] = "MsgD"     ,
  [0x75] = "MsgD"     ,
  [0x76] = "MsgD"     ,
  [0x77] = "MsgD"     ,
  [0x0A] = "Cpl"      ,
  [0x4A] = "CplD"     ,
  [0x0B] = "CplLk"    ,
  [0x4B] = "CplDLk"   ,
  [0x4C] = "FetchAdd" ,
  [0x6C] = "FetchAdd" ,
  [0x4D] = "Swap"     ,
  [0x6D] = "Swap"     ,
  [0x4E] = "CAS"      ,
  [0x6E] = "CAS"      ,
  [0x80] = "LPrfx"    ,
  [0x81] = "LPrfx"    ,
  [0x82] = "LPrfx"    ,
  [0x83] = "LPrfx"    ,
  [0x84] = "LPrfx"    ,
  [0x85] = "LPrfx"    ,
  [0x86] = "LPrfx"    ,
  [0x87] = "LPrfx"    ,
  [0x88] = "EPrfx"    ,
  [0x89] = "EPrfx"    ,
  [0x8A] = "EPrfx"    ,
  [0x8B] = "EPrfx"    ,
  [0x8C] = "EPrfx"    ,
  [0x8D] = "EPrfx"    ,
  [0x8E] = "EPrfx"    ,
  [0x8F] = "EPrfx"
}

f.tlp_tlpType = ProtoField.uint8("pcie.tlp.tlpType", "TLP Type", base.HEX, tlpType)

f.tlp_rsvd1   = ProtoField.new("Reserved1", "pcie.tlp.reserved1", ftypes.UINT8, nil, base.NONE)
f.tlp_tclass  = ProtoField.new("Tclass", "pcie.tlp.tclass", ftypes.UINT8, nil, base.HEX)
f.tlp_rsvd2   = ProtoField.new("Reserved2", "pcie.tlp.reserved2", ftypes.UINT8, nil, base.NONE)
f.tlp_digest  = ProtoField.new("Digest", "pcie.tlp.digest", ftypes.UINT8, nil, base.HEX)
f.tlp_poison  = ProtoField.new("Poison", "pcie.tlp.poison", ftypes.UINT8, nil, base.HEX)
f.tlp_attr    = ProtoField.new("Attr", "pcie.tlp.attr", ftypes.UINT8, nil, base.HEX)
f.tlp_rsvd3   = ProtoField.new("Reserved3", "pcie.tlp.reserved3", ftypes.UINT8, nil, base.NONE)
f.tlp_length  = ProtoField.new("Length", "pcie.tlp.length", ftypes.UINT8, nil, base.DEC)
f.tlp_RsvdLen = ProtoField.new("Reserved", "pcie.tlp.reservedLength", ftypes.UINT8, nil, base.HEX)
f.tlp_reqid   = ProtoField.new("Request ID", "pcie.tlp.reqid", ftypes.UINT8, nil, base.HEX)
f.tlp_tag     = ProtoField.new("Tag", "pcie.tlp.tag", ftypes.UINT8, nil, base.HEX)
f.tlp_lastbe  = ProtoField.new("LastBE", "pcie.tlp.lastbe", ftypes.UINT8, nil, base.HEX)
f.tlp_firstbe = ProtoField.new("FirstBE", "pcie.tlp.firstbe", ftypes.UINT8, nil, base.HEX)
f.tlp_addr    = ProtoField.new("Address", "pcie.tlp.addr", ftypes.UINT8, nil, base.HEX)
f.tlp_rsvd4   = ProtoField.new("Reserved4", "pcie.tlp.reserved4", ftypes.UINT8, nil, base.NONE)
f.tlp_rsvd4   = ProtoField.new("Reserved4", "pcie.tlp.reserved4", ftypes.UINT8, nil, base.NONE)
f.tlp_rsvd4   = ProtoField.new("Reserved4", "pcie.tlp.reserved4", ftypes.UINT8, nil, base.NONE)
f.tlp_rsvd4   = ProtoField.new("Reserved4", "pcie.tlp.reserved4", ftypes.UINT8, nil, base.NONE)
f.tlp_rsvd4   = ProtoField.new("Reserved4", "pcie.tlp.reserved4", ftypes.UINT8, nil, base.NONE)
f.tlp_4thDW_HDR   = ProtoField.new("4thDW_HDR", "pcie.tlp.4thDW_HDR", ftypes.BYTES, nil, base.NONE)
f.tlp_payload = ProtoField.new("Payload", "pcie.tlp.payload", ftypes.BYTES, nil, base.NONE)
f.tlp_valPayload = ProtoField.new("Valid Payload", "pcie.tlp.validPayload", ftypes.BYTES, nil, base.NONE)
f.tlp_analysisFlag = ProtoField.new("Analysis Flag", "pcie.tlp.analysis.flag", ftypes.NONE, nil, base.TEXT)

function pcie_proto.dissector(buffer, pinfo, tree)
-- Dissector.get("infiniband"):call(buffer,pinfo,tree)
  local tlpOffset = 4
  local cnt = 0

  local subtree = tree:add(pcie_proto, buffer(tlpOffset+0, buffer:len()-tlpOffset))

  -- iterate thorugh buffer until at least a minimal TLP does not fit anymore
  while tlpOffset+12 < buffer:len() do
    pinfo.cols.protocol = "PCIe TLP"

    local tlpStart = tlpOffset
    local tlpPayloadLength = buffer(tlpOffset+ 2,2):bitfield(6,10) * 4
    if tlpPayloadLength == 0 then
      --tlpPayloadLength = 1024*4
    end
    local tlpLength = 0
    local tlpHeaderLength = 0
    local tlpHasPayload = false
    local tlpHas4thHeaderDW = false
    local isMemReq = false

    local tlp_subtree = subtree:add(pcie_proto, buffer(tlpOffset+0, buffer:len()-tlpOffset), "PCIe Transaction Layer Packet")

    local t_tree = tlp_subtree:add(f.tlp_tlpType, buffer(tlpOffset+ 0,1)):set_generated()

    if (tlpType == 0x40 or tlpType == 0x60 or tlpType == 0x00 or tlpType == 0x20) then
      isMemReq = true
    end

    local tlpType = buffer(tlpOffset+ 0,1):uint()
    t_tree:add(f.tlp_fmt,     buffer(tlpOffset+ 0,1), buffer(tlpOffset+ 0,1):bitfield(0, 3))
    t_tree:add(f.tlp_type,    buffer(tlpOffset+ 0,1), buffer(tlpOffset+ 0,1):bitfield(3, 5))
    tlp_subtree:add(f.tlp_rsvd1,   buffer(tlpOffset+ 1,1), buffer(tlpOffset+ 1,1):bitfield(0, 1))
    tlp_subtree:add(f.tlp_tclass,  buffer(tlpOffset+ 1,1), buffer(tlpOffset+ 1,1):bitfield(1, 3))
    tlp_subtree:add(f.tlp_rsvd2,   buffer(tlpOffset+ 1,1), buffer(tlpOffset+ 1,1):bitfield(4, 4))
    tlp_subtree:add(f.tlp_digest,  buffer(tlpOffset+ 2,1), buffer(tlpOffset+ 2,1):bitfield(0, 1))
    tlp_subtree:add(f.tlp_poison,  buffer(tlpOffset+ 2,1), buffer(tlpOffset+ 2,1):bitfield(1, 1))
    tlp_subtree:add(f.tlp_attr,    buffer(tlpOffset+ 2,1), buffer(tlpOffset+ 2,1):bitfield(2, 2))
    tlp_subtree:add(f.tlp_rsvd3,   buffer(tlpOffset+ 2,1), buffer(tlpOffset+ 2,1):bitfield(4, 2))

    -- length field is only valid for TLPs with Payload
    if (tlpType >= 0x70 and tlpType <= 0x70) or (tlpType == 0x4A) or (tlpType == 0x4B)  -- MsgD, CplD, CplDLk,
      or (tlpType == 0x40 or tlpType == 0x60) or (tlpType == 0x44) -- MWr
    then
      tlp_subtree:add(f.tlp_length,  buffer(tlpOffset+ 2,2), buffer(tlpOffset+ 2,2):bitfield(6,10))
      tlpHasPayload = true
    else
      tlpPayloadLength = 0
      tlp_subtree:add(f.tlp_RsvdLen,  buffer(tlpOffset+ 2,2), buffer(tlpOffset+ 2,2):bitfield(6,10))
    end

    tlp_subtree:add(f.tlp_reqid,   buffer(tlpOffset+ 4,2), buffer(tlpOffset+ 4,2):bitfield(0,16))
    tlp_subtree:add(f.tlp_tag,     buffer(tlpOffset+ 6,1), buffer(tlpOffset+ 6,1):bitfield(0, 8))

    local isContiguos = true
    -- length LBE & FBE are only valid for TLPs with payload
    local firstBe = buffer(tlpOffset+ 7,1):bitfield(4, 4)
    local lastBe = buffer(tlpOffset+ 7,1):bitfield(0, 4)
    if tlpHasPayload then
      firstBe_tree = tlp_subtree:add(f.tlp_firstbe, buffer(tlpOffset+ 7,1), buffer(tlpOffset+ 7,1):bitfield(4, 4))
      if (isMemReq)
        and (tlpPayloadLength == 1*4) and (lastBe == 0 or firstBe == 0)
      then
        ;
      elseif tlpPayloadLength > 1*4 and firstBe == 0 then
        --firstBe_tree:add_expert_info(PI_MALFORMED, PI_ERROR, "invalid value")
        --tlp_subtree:add(f.tlp_analysisFlag, "invalid value")
      end

      lastBe_tree = tlp_subtree:add(f.tlp_lastbe,  buffer(tlpOffset+ 7,1), buffer(tlpOffset+ 7,1):bitfield(0, 4))

      if (isMemReq)
        and (tlpPayloadLength == 1*4) and (lastBe == 0 or firstBe == 0)
      then
        ;
      elseif tlpPayloadLength == 1*4 and lastBe ~= 0 then
        --lastBe_tree:add_expert_info(PI_MALFORMED, PI_ERROR, "invalid value")
        --tlp_subtree:add(f.tlp_analysisFlag, "invalid value")
      elseif tlpPayloadLength > 1*4 and lastBe == 0 then
        --lastBe_tree:add_expert_info(PI_MALFORMED, PI_ERROR, "invalid value")
        --tlp_subtree:add(f.tlp_analysisFlag, "invalid value")
      end
      -- check for coniguous BE rules
      if (isMemReq)
        and ((tlpPayloadLength >= 3*4) or ((tlpPayloadLength == 2*4) and (buffer(tlpOffset+ 8,4):bitfield(0,30) % 4 ~= 0)))
      then
        local lastVal = buffer(tlpOffset+ 7,1):bitfield(0, 1)
        for i=0,1,3 do
          if buffer(tlpOffset+ 7,1):bitfield(i, 1) ~= lastVal then
            if lastVal == 1 then
              isContiguos = false
              lastBe_tree:add_expert_info(PI_MALFORMED, PI_ERROR, "discontiguous LBE ")
              tlp_subtree:add(f.tlp_analysisFlag, "discontiguous LBE")
            end
          end
        end
        local lastVal = buffer(tlpOffset+ 7,1):bitfield(4, 1)
        for i=4,1,7 do
          if buffer(tlpOffset+ 7,1):bitfield(i, 1) ~= lastVal then
            if lastVal == 0 then
              isContiguos = false
              firstBe_tree:add_expert_info(PI_MALFORMED, PI_ERROR, "discontiguous FBE")
              tlp_subtree:add(f.tlp_analysisFlag, "discontiguous FBE")
            end
          end
        end
      end


    end

    tlp_subtree:add(f.tlp_addr,    buffer(tlpOffset+ 8,4), buffer(tlpOffset+ 8,4):bitfield(0,30))
    tlp_subtree:add(f.tlp_rsvd4,   buffer(tlpOffset+11,1), buffer(tlpOffset+11,1):bitfield(6, 2))

    tlp_subtree:append_text(" (Payload Length: " .. tostring(tlpPayloadLength) .. " at tlpOffset: " .. tostring(tlpOffset) .. ")")

    -- header processing
    -- 3DW_NO_DAT
    if buffer(tlpOffset+ 0,1):bitfield(0, 3) == 0 then
      tlpHeaderLength = 12
      tlp_subtree:set_len(tlpHeaderLength)
--       tlpPayloadLength = 0
    -- 4DW_NO_DATA
    elseif buffer(tlpOffset+ 0,1):bitfield(0, 3) == 1 then
      tlpHeaderLength = 16
      tlpHas4thHeaderDW = true
      tlp_subtree:set_len(tlpHeaderLength)
--       tlpPayloadLength = 0
    -- 3DW_DATA
    elseif buffer(tlpOffset+ 0,1):bitfield(0, 3) == 2 then
      tlpHeaderLength = 12
--       tlpHasPayload = true
    -- 4DW_DATA
    elseif buffer(tlpOffset+ 0,1):bitfield(0, 3) == 3 then
      tlpHeaderLength = 16
      tlpHas4thHeaderDW = true
--       tlpHasPayload = true
    elseif buffer(tlpOffset+ 0,1):bitfield(0, 3) == 4 then
      tlp_subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "encountered TLP header - Unsupported!")
    else
      tlpHeaderLength = 0
      tlpLength = buffer:len()
      tlp_subtree:set_len(tlpLength)
      tlp_subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "unknown FMT value")
    end
    if tlpHas4thHeaderDW then
       tlp_subtree:add(f.tlp_4thDW_HDR, buffer(tlpOffset+12, 4))
    end

    local address = 0
    -- TODO handle address extraction and check for 4 kiB boundary crossing
    -- chec kfor 64  / 32 bit memory addresses

    tlpLength = tlpPayloadLength + tlpHeaderLength
    tlp_subtree:set_len(tlpLength)
    subtree:set_len(tlpLength)

    -- payload processing
    if tlpHasPayload then
      local tlpError = false
      local originalPayloadLength = tlpPayloadLength
      if tlpOffset+tlpPayloadLength>buffer:len() then
        tlpPayloadLength = buffer:len()-tlpOffset-tlpHeaderLength
        tlpError = true
      end

      tlp_pl = tlp_subtree:add(f.tlp_payload, buffer(tlpOffset+tlpHeaderLength,tlpPayloadLength))
      local valPrecut = 0
      local valPostcut = 0
      if isContiguos then
        for i=0,3 do
          if buffer(tlpOffset+ 7,1):bitfield(4+i, 1) == 0 then
            valPrecut = valPrecut + 1
          end
          if buffer(tlpOffset+ 7,1):bitfield(i, 1) == 0 then
            if tlpPayloadLength > 1*4 then
              valPostcut = valPostcut + 1
            end
          end
        end
      end
      tlp_pl:add(f.tlp_valPayload, buffer(tlpOffset+tlpHeaderLength+valPrecut,tlpPayloadLength-valPostcut))
      if tlpError then
        tlp_pl:add_expert_info(PI_MALFORMED, PI_ERROR, "Partial TLP: expected length " .. tostring(originalPayloadLength) .. "B, got " .. tostring(tlpPayloadLength) .. "B")
        tlp_subtree:add(f.tlp_analysisFlag, "Partial TLP")
      end
    end

    tlpOffset = tlpOffset + tlpLength

    -- padding may occur after each TLP, also in the end of a segment
    -- padding is bound to 16 Bytes boundaries or less if the segment ends before
    local paddingStart = tlpOffset
    local paddingEnd = tlpOffset+16

    paddingLength = 32-math.fmod(tlpLength,32)
    paddingEnd = paddingStart + paddingLength

    if paddingEnd > buffer:len() then
      paddingEnd = buffer:len()
    end
    local paddingPresent = true
    if paddingStart >= paddingEnd then
      paddingPresent = false
      subtree:append_text(" FALSE")
    end

    if paddingPresent then
      padding_subtree = subtree:add(padding_proto, buffer(paddingStart, paddingEnd-paddingStart), "Padding")
      padding_subtree:add(p.padding_length, paddingEnd-paddingStart):set_generated()
      padding_subtree:add(p.padding_data, buffer(paddingStart, paddingEnd-paddingStart))
    end
    tlpOffset = paddingEnd

  end

end

DissectorTable.get("udp.port"):add("21212", pcie_proto)
--DissectorTable.get("udp.port"):add_for_decode_as(pcie_proto)
--DissectorTable.get("udp.port"):add("4791", pcie_proto)
