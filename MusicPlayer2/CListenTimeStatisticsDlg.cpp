﻿// CListenTimeStatisticsDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MusicPlayer2.h"
#include "CListenTimeStatisticsDlg.h"
#include "afxdialogex.h"


// CListenTimeStatisticsDlg 对话框

IMPLEMENT_DYNAMIC(CListenTimeStatisticsDlg, CDialog)

CListenTimeStatisticsDlg::CListenTimeStatisticsDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_LISTEN_TIME_STATISTICS_DLG, pParent)
{

}

CListenTimeStatisticsDlg::~CListenTimeStatisticsDlg()
{
}

void CListenTimeStatisticsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list_ctrl);
}

static bool ByListenTime(const SongInfo& a, const SongInfo& b)
{
	return a.listen_time > b.listen_time;
}


void CListenTimeStatisticsDlg::ShowData()
{
	vector<SongInfo> data_list;

	//从所有歌曲信息中查找累计听的时间5秒以上的并添加到vector
	for (const auto& data : theApp.m_song_data)
	{
		if (data.second.listen_time >= 5)
		{
			data_list.push_back(data.second);
			data_list.back().file_name = data.first;
		}
	}

	//按累计收听时间从大到小排序
	std::sort(data_list.begin(), data_list.end(), ByListenTime);
	
	//将vector中的数据显示到列表中
	m_list_ctrl.DeleteAllItems();

	int index = 0;
	for (const auto& data : data_list)
	{
		m_list_ctrl.InsertItem(index, std::to_wstring(index + 1).c_str());
		m_list_ctrl.SetItemText(index, 1, (data.artist + L" - " + data.title).c_str());
		m_list_ctrl.SetItemText(index, 2, data.file_name.c_str());

		int hour, min, sec;
		sec = data.listen_time % 60;
		min = data.listen_time / 60 % 60;
		hour = data.listen_time / 60 / 60;
		CString str;
		str.Format(_T("%d:%.2d:%.2d"), hour, min, sec);
		m_list_ctrl.SetItemText(index, 3, str);

		m_list_ctrl.SetItemText(index, 4, data.lengh.time2str().c_str());
		double times = static_cast<double>(data.listen_time) / data.lengh.time2int() * 1000;
		str.Format(_T("%.2f"), times);
		m_list_ctrl.SetItemText(index, 5, str);

		index++;
	}
}


BEGIN_MESSAGE_MAP(CListenTimeStatisticsDlg, CDialog)
	ON_BN_CLICKED(IDC_EXPORT_BUTTON, &CListenTimeStatisticsDlg::OnBnClickedExportButton)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


// CListenTimeStatisticsDlg 消息处理程序


BOOL CListenTimeStatisticsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	//获取初始时窗口的大小
	CRect rect;
	GetWindowRect(rect);
	m_min_size.cx = rect.Width();
	m_min_size.cy = rect.Height();

	//初始化列表
	m_list_ctrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP);
	m_list_ctrl.InsertColumn(0, CCommon::LoadText(IDS_NUMBER), LVCFMT_LEFT, theApp.DPI(40));
	m_list_ctrl.InsertColumn(1, CCommon::LoadText(IDS_TRACK), LVCFMT_LEFT, theApp.DPI(150));
	m_list_ctrl.InsertColumn(2, CCommon::LoadText(IDS_PATH), LVCFMT_LEFT, theApp.DPI(130));
	m_list_ctrl.InsertColumn(3, CCommon::LoadText(IDS_LISTEN_TIME), LVCFMT_LEFT, theApp.DPI(60));
	m_list_ctrl.InsertColumn(4, CCommon::LoadText(IDS_LENGTH), LVCFMT_LEFT, theApp.DPI(60));
	m_list_ctrl.InsertColumn(5, CCommon::LoadText(IDS_LISTEN_TIMES), LVCFMT_LEFT, theApp.DPI(60));

	ShowData();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CListenTimeStatisticsDlg::OnBnClickedExportButton()
{
	// TODO: 在此添加控件通知处理程序代码

	//生成导出的csv文本
	CString str;
	int list_size = m_list_ctrl.GetItemCount();
	str += CCommon::LoadText(IDS_NUMBER);
	str += _T(',');
	str += CCommon::LoadText(IDS_TRACK);
	str += _T(',');
	str += CCommon::LoadText(IDS_PATH);
	str += _T(',');
	str += CCommon::LoadText(IDS_LISTEN_TIME);
	str += _T(',');
	str += CCommon::LoadText(IDS_LENGTH);
	str += _T(',');
	str += CCommon::LoadText(IDS_LISTEN_TIMES);
	str += _T('\n');

	for (int i = 0; i < list_size; i++)
	{
		str += m_list_ctrl.GetItemText(i, 0);
		str += _T(',');
		str += m_list_ctrl.GetItemText(i, 1);
		str += _T(',');
		str += m_list_ctrl.GetItemText(i, 2);
		str += _T(',');
		str += m_list_ctrl.GetItemText(i, 3);
		str += _T('\n');
	}

	//弹出保存对话框

	CString filter = CCommon::LoadText(IDS_LISTEN_TIME_FILE_DLG_FILTER);
	CString file_name = CCommon::LoadText(IDS_LISTEN_TIME);
	CString str_cur_date;
	SYSTEMTIME cur_time;
	GetLocalTime(&cur_time);
	str_cur_date.Format(_T("_%.4d-%.2d-%.2d"), cur_time.wYear, cur_time.wMonth, cur_time.wDay);
	file_name += str_cur_date;

	// 构造保存文件对话框
	CFileDialog fileDlg(FALSE, _T("csv"), file_name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, this);
	// 显示保存文件对话框
	if (IDOK == fileDlg.DoModal())
	{
		ofstream out_put{ fileDlg.GetPathName().GetString() };
		out_put << CCommon::UnicodeToStr(wstring(str), CodeType::ANSI);
	}
}


void CListenTimeStatisticsDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//限制窗口最小大小
	lpMMI->ptMinTrackSize.x = m_min_size.cx;		//设置最小宽度
	lpMMI->ptMinTrackSize.y = m_min_size.cy;		//设置最小高度

	CDialog::OnGetMinMaxInfo(lpMMI);
}
