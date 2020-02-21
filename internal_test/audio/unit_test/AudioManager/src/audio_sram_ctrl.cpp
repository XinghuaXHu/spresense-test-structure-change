#include "MemMgrTypes.h"
#include "audio_sram_ctrl.h"

namespace AudioSramCtrl {

uint8_t	SramOnTile_Tile_No;
uint8_t	SramRetTile_Tile_No;
uint8_t	SramOffTile_Tile_No;
MemMgrLite::NumLayout	 SramChangeStatusby_Layout_No;
MemMgrLite::NumLayout	 SramOnby_Layout_No;
MemMgrLite::NumLayout	 SramOffby_Layout_No;
int	Sram_Status;

void SramOnTile(uint8_t tile_no)
{
	SramOnTile_Tile_No = tile_no;
}
void SramRetTile(uint8_t tile_no)
{
	SramRetTile_Tile_No = tile_no;
}
void SramOffTile(uint8_t tile_no, int status)
{
	SramOffTile_Tile_No = tile_no;
}
void SramOnLastTile()
{
	SramOnTile(3);
}
void SramChangeStatusbyMemLayoutNo(MemMgrLite::NumLayout layout_no, int status)
{
	SramChangeStatusby_Layout_No = layout_no;
	Sram_Status = status;
}
void SramOnbyMemLayoutNo(MemMgrLite::NumLayout layout_no)
{
	SramOnby_Layout_No = layout_no;
}
void SramOffbyMemLayoutNo(MemMgrLite::NumLayout layout_no)
{
	SramOffby_Layout_No = layout_no;
}

}
