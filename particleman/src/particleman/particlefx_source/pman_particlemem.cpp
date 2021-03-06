/**
* Copyright (C) 2017, 2010kohtep
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "pman_particlemem.h"
#include "pman_main.h"
#include "maintypes.h"

long CMiniMem::m_lMemoryPoolSize = 0;
long CMiniMem::m_lMemoryBlockSize = 0;
long CMiniMem::m_lMaxBlocks = 0;
CMiniMem *CMiniMem::_instance = nullptr;

CMiniMem::CMiniMem(long lMemoryPoolSize, long lMaxBlockSize)
{
	// m_lMemoryPoolSize = lMemoryPoolSize;
	// m_lMemoryBlockSize = lMaxBlockSize;
	// m_lMaxBlocks = lMemoryPoolSize / sizeof MemoryBlock;
	//
	// if (lMemoryPoolSize / 16 != 0)
	// {
	// 	int i = 0;
	//
	// 	do
	// 	{
	// 		while (true)
	// 		{
	// 			auto block = new MemoryBlock(m_lMemoryBlockSize);
	// 			block->next = nullptr;
	// 			block->prev = nullptr;
	//
	// 			// typedef std::vector<MemoryBlock *> VectorOfMemoryBlocks;
	// 			// typedef VectorOfMemoryBlocks::iterator MemoryBlockIterator;
	//
	// 			// v7 = CMiniMem::m_vecMemoryPool.baseclass_0._M_impl._M_finish;
	//
	// 			if (true /* v7 == CMiniMem::m_vecMemoryPool.baseclass_0._M_impl._M_end_of_storage */)
	// 			{
	//
	// 			}
	// 			else
	// 			{
	// 				// std::allocator<MemoryBlock*>::pointer v8;
	// 			}
	// 		}
	//
	// 		i++;
	// 	}
	// }
}

CMiniMem::CMiniMem(long lMaxBlockSize)
{

}

CMiniMem::~CMiniMem()
{

}

UNTESTED char* CMiniMem::AllocateFreeBlock()
{
	auto pFM = m_FreeMem;
	char* temp = nullptr;

	if (!m_FreeMem)
		return temp;

	m_FreeMem = m_FreeMem->next;
	if (m_FreeMem)
		m_FreeMem->prev = 0;

	auto pHead = m_ActiveMem.Front();
	pFM->next = 0;
	pFM->prev = 0;
	m_ActiveMem.SetFront(pFM);

	if (pHead)
	{
		pFM->next = pHead;
		pHead->prev = pFM;
	}

	temp = pFM->Memory();
	return temp;
}

UNTESTED char* CMiniMem::newBlock(void)
{
	if (!m_FreeMem)
	{
	Exit:
		gEngfuncs.Con_DPrintf("Particleman is out of memory, too many particles");
		return nullptr;
	}

	m_FreeMem = m_FreeMem->next;
	if (m_FreeMem)
		m_FreeMem->prev = nullptr;

	auto pHead = m_ActiveMem.Front();
	m_FreeMem->next = nullptr;
	m_FreeMem->prev = nullptr;

	if (pHead)
	{
		m_FreeMem->next = nullptr;
		pHead->prev = nullptr;
	}

	auto pRes = m_FreeMem->Memory();
	if (!pRes)
		goto Exit;

	return pRes;
}

UNTESTED void CMiniMem::deleteBlock(MemoryBlock *p)
{
	if (p == m_ActiveMem.Front())
	{
		m_ActiveMem.SetFront(p->next);
		if (m_ActiveMem.Front())
			m_ActiveMem.Front()->prev = nullptr;
	}
	else
	{
		auto pPrev = p->prev;
		auto pNext = p->next;

		if (pPrev)
			pPrev->next = pNext;

		if (pNext)
			pNext->prev = pPrev;
	}

	auto pFM = m_FreeMem;

	p->next = 0;
	p->prev = 0;
	m_FreeMem = p;

	if (pFM)
	{
		p->next = pFM;
		p->prev = 0;
		pFM->prev = p;
	}
	else
	{
		p->next = 0;
		p->prev = 0;
	}
}

UNTESTED long CMiniMem::PercentUsed(void)
{
	auto uCount = 0u;
	auto pMem = m_FreeMem;

	while (pMem)
	{
		uCount++;
		pMem = pMem->next;
	}

	return 100 * uCount / m_lMaxBlocks;
}

int __cdecl ParticleSortFn(const void* p1, const void* p2)
{
	auto fp1Dist = ((visibleparticles_t *)p1)->pVisibleParticle->GetPlayerDistance();
	auto fp2Dist = ((visibleparticles_t *)p2)->pVisibleParticle->GetPlayerDistance();

	if (fp2Dist >= fp1Dist)
		return fp2Dist != fp1Dist;
	else
		return -1;
}

UNTESTED void CMiniMem::ProcessAll(void)
{
#if 0
	auto fClTime = gEngfuncs.GetClientTime();
	auto pHead = m_ActiveMem.Front();

	this->m_iTotalParticles = 0;
	this->m_iTotalParticles = 0;
	memset(this->m_pVisibleParticles, 0, m_lMaxBlocks * sizeof visibleparticles_t);

	while (pHead)
	{
		auto pParticle = (CCoreTriangleEffect *)pHead->Memory();

		if (pParticle)
		{
			auto pData = pHead->Memory();
			this->m_iTotalParticles++;

			if (pParticle->CheckVisibility())
			{
				Vector plyOrg = gEngfuncs.GetLocalPlayer()->origin; // Player origin
				Vector prtOrg = pParticle->m_vOrigin;               // Particle origin

				auto fDiffX = plyOrg.x - prtOrg.x;
				auto fDiffY = plyOrg.y - prtOrg.y;
				auto fDiffZ = plyOrg.z - prtOrg.z;
				auto fDistance = (fDiffX * fDiffX) + (fDiffY * fDiffY) + (fDiffZ + fDiffZ);

				pParticle->SetPlayerDistance(fDistance);
				this->m_pVisibleParticles->pVisibleParticle = pParticle;
				CMiniMem::Instance()->m_iParticlesDrawn++;
			}

			if (!IsGamePaused())
				pParticle->Think(fClTime);

			if (pParticle->m_flDieTime == 0.0f || pParticle->m_flDieTime > fClTime)
				pHead = pHead->next;
			else
			{
				pParticle->Die();
				auto pCurHead = pHead;
				pHead = pHead->next;
				CMiniMem::deleteBlock(pHead);
			}
		}
	}

	qsort(this->m_pVisibleParticles, this->m_iParticlesDrawn, sizeof visibleparticles_t, ParticleSortFn);

	if (this->m_iParticlesDrawn <= 0)
	{
		g_flOldTime = fClTime;
	}
	else
	{
		for (int i = 0; i < this->m_iParticlesDrawn; i++)
			this->m_pVisibleParticles[i].pVisibleParticle->Draw();

		g_flOldTime = fClTime;
	}
#endif
}

UNTESTED void CMiniMem::Reset(void)
{
	auto pHead = m_ActiveMem.Front();

	if (pHead)
	{
		auto pHeadNext = pHead->next;
		while (pHeadNext)
		{
			pHeadNext->prev = nullptr;
			pHead->next = nullptr;
			pHead->prev = nullptr;
			pHead = pHeadNext;

			pHeadNext = pHeadNext->next;
		}

		pHead->next = nullptr;
		pHead->prev = nullptr;
		pHead = nullptr;
	}

	auto pFM = m_FreeMem;
	if (pFM)
	{
		auto pFMNext = m_FreeMem->next;

		while (pFMNext)
		{
			pFMNext->prev = nullptr;
			pFM->next = nullptr;
			pFM->prev = nullptr;
			pFM = pFMNext;

			pFMNext = pFMNext->next;
		}

		pFM->next = nullptr;
		pFM->prev = nullptr;
		m_FreeMem = nullptr;
	}

	// for (auto &&mem : CMiniMem::GetVecMemoryPool())
	{
		// TODO: finish vector here
	}
}

int CMiniMem::ApplyForce(Vector vOrigin, Vector vDirection, float flRadius, float flStrength)
{
	return 0;
}

// TODO: move it to another place
unsigned int g_lMaxParticleClassSize;

UNTESTED CMiniMem* CMiniMem::Instance(void)
{
	auto instance = _instance;

	if (instance)
		return instance;

	instance = new CMiniMem(30000, g_lMaxParticleClassSize);
	return instance;
}

UNTESTED long CMiniMem::MaxBlockSize(void)
{
	return m_lMemoryBlockSize;
}

UNTESTED bool CMiniMem::CheckSize(int iSize)
{
	if (_instance)
		return iSize <= m_lMemoryBlockSize;

	auto instance = new CMiniMem(g_lMaxParticleClassSize);
	_instance = instance;

	return iSize <= m_lMemoryBlockSize;
}

UNTESTED void CMiniMem::Shutdown(void)
{
	if (_instance)
		delete _instance;
}