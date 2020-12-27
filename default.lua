local WHITE = 0
local BLACK = 1

local KING = 0
local PAWN = 1
local KNIGHT = 2
local BISHOP = 3
local ROOK = 4
local QUEEN = 5

local pieceValue = { 0, 100, 300, 300, 500, 900 }

function getMaterialValue(color)
    local val = 0
    local np = getPieceCount(color)
    for i=0, np-1 do
        local alive, type, square = getPieceInfo(color, i)
        if alive then
            val = val + pieceValue[type + 1]
        end
    end
    return val
end

function evaluate()
    local score = 0
    
    score = score + getMaterialValue(WHITE) + getMoveCount(WHITE)
    score = score - getMaterialValue(BLACK) - getMoveCount(BLACK)
   
    return score
end