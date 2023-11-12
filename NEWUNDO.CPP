
	UINT * m_pFree;
	UINT m_nFree;
	UINT i;
	for (i = 0; i < MAX; i++)
		m_pFree[i] = i;
	m_nFree = i;
	m_nStack = 0;
	m_nBlockSize = 100000;
	m_nMaxStack = 0;

/*

	there is an extra buffer
	depending on tool
	this buffer has a copy of the canvas

	before start draw they are the same
	we remember minx, miny, etc
	on stop draw the changes are pushed onto a stack
*/
	we now have x,y, w, h of changes

stop draw
psuh data onto statck
and copy from canvas to reference




	Push(x,y,w,h)

	UINT act_w = w * 3;

	UINT LinesPerBlock = BlockSize / act_w;
	UINT blocks = (h + LinesPerBlock - 1) / LInesPerBlock;

	for (; blocks < m_nFree;) // need to free a block
		{
		q = m_pStack[0].blocks
		z = m_pStack[0].first;
		UINT i;
		for (i = 0; i < q; i++)
			{
			s = (z + i) % blocks;
			m_pFree[m_nFree++] = s;
			}
		m_nStack--;
		for (i = 0; i < m_nStack;i++)
			m_pStack[i] = m_pStack[i+1];
		}
	m_pStack[m_nStack].blocks= blocks;
	m_pStack[m_nStack].first = m_pFree[m_nFree - 1];
	m_nMaxStack = m_nStack++;
	UINT i;
	BYTE * pDst;
	BYTE * pSrc = scree + y * m_pitch + x * 3;
	for (i = 0; i < h; i++,pSrc += m_pitch;)
		{
		if (!(i % LinesPerBlock))
			pDst = m_pFree[--m_nFree];
		UINT j;
		for (j = 0; j < actw; j++)
			pDSt[j] = pSrc[j];
		}

if (nStack < m_nMaxStack) allow redo

if (nStack) allow undo


undo
	xchg refernce with stack
	copy area from ref to canvas
	sp--;

redo
	same as undo but sp++;
