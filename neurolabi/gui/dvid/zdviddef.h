#ifndef ZDVIDDEF_H
#define ZDVIDDEF_H

namespace ZDvid {
enum ELabelIndexType {
  INDEX_POST_SYN, INDEX_PRE_SYN, INDEX_GAP, INDEX_ALL_SYN,
  INDEX_NOTE, INDEX_VOXEL
};

enum ENodeStatus {
  NODE_NORMAL, NODE_INVALID, NODE_OFFLINE, NODE_LOCKED
};


static const int DEFAULT_ROI_BLOCK_SIZE = 32;

}

#endif // ZDVIDDEF_H