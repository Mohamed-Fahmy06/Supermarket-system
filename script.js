document.addEventListener('DOMContentLoaded', () => {
    // Elements
    const loginScreen = document.getElementById('login-screen');
    const dashboardScreen = document.getElementById('dashboard-screen');
    const loginForm = document.getElementById('login-form');
    const userIdInput = document.getElementById('user-id');
    const logoutBtn = document.getElementById('logout-btn');
    const navLinks = document.getElementById('nav-links');
    const userRoleTitle = document.getElementById('user-role-title');
    const userIdDisplay = document.getElementById('user-id-display');
    const currentDate = document.getElementById('current-date');
    const contentArea = document.getElementById('content-area');
    const managerStats = document.getElementById('manager-stats');
    const pageTitle = document.getElementById('page-title');

    // Set Date
    const options = { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' };
    currentDate.textContent = new Date().toLocaleDateString('ar-EG', options);

    // Role Mapping
    const roleMapping = {
        0: { role: 'manager', title: 'المدير' },
        1: { role: 'supervisor', title: 'مشرف' },
        2: { role: 'casher', title: 'كاشير' },
        3: { role: 'customer', title: 'عميل' }
    };

    // Menus Configuration
    const menus = {
        manager: [
            { icon: 'fa-home', text: 'الرئيسية', action: 'home' },
            { icon: 'fa-box', text: 'المخزون', action: 'stock' },
            { icon: 'fa-chart-bar', text: 'التقارير', action: 'reports' },
            { icon: 'fa-users', text: 'إدارة الحسابات', action: 'accounts' }
        ],
        supervisor: [
            { icon: 'fa-home', text: 'الرئيسية', action: 'home' },
            { icon: 'fa-box', text: 'عرض المخزون', action: 'stock' }
        ],
        casher: [
            { icon: 'fa-home', text: 'الرئيسية', action: 'home' },
            { icon: 'fa-box', text: 'استعلام مخزون', action: 'stock' }
        ],
        customer: [
            { icon: 'fa-home', text: 'الرئيسية', action: 'home' },
            { icon: 'fa-store', text: 'تصفح المنتجات', action: 'stock' }
        ]
    };

    // Data Verification
    if (typeof inventoryData === 'undefined') {
        console.error("Data file not found. Please run the C system first.");
        alert("تنبيه: لم يتم العثور على ملف البيانات (data.js). يرجى تشغيل نظام الـ C وتصدير البيانات أولاً.");
    }

    // Login Handler
    loginForm.addEventListener('submit', (e) => {
        e.preventDefault();
        const id = userIdInput.value.trim();
        if(!id) return;

        // Find user in accountsData (exported from C)
        const userAccount = typeof accountsData !== 'undefined' ? accountsData.find(a => a.id === id) : null;
        
        if (!userAccount && id !== '#001') {
            alert("المعرف غير صحيح أو غير مسجل في النظام.");
            return;
        }

        const roleInfo = userAccount ? roleMapping[userAccount.role] : roleMapping[0];
        
        // Update UI
        userRoleTitle.textContent = roleInfo.title;
        userIdDisplay.textContent = id;
        
        // Render Menu
        renderMenu(roleInfo.role);

        // Show Dashboard
        loginScreen.classList.remove('active');
        dashboardScreen.classList.add('active');

        // Initial Page
        renderHome(roleInfo.role);
    });

    // Logout Handler
    logoutBtn.addEventListener('click', () => {
        dashboardScreen.classList.remove('active');
        loginScreen.classList.add('active');
        userIdInput.value = '';
    });

    // Render Menu Links
    function renderMenu(role) {
        navLinks.innerHTML = '';
        const items = menus[role];
        
        items.forEach((item, index) => {
            const li = document.createElement('li');
            const a = document.createElement('a');
            a.href = '#';
            a.innerHTML = `<i class="fas ${item.icon}"></i> ${item.text}`;
            if(index === 0) a.classList.add('active');
            
            a.addEventListener('click', (e) => {
                e.preventDefault();
                document.querySelectorAll('.nav-links a').forEach(el => el.classList.remove('active'));
                a.classList.add('active');
                
                pageTitle.textContent = item.text;
                if(item.action === 'stock') renderStock();
                else if(item.action === 'home') renderHome(role);
                else if(item.action === 'accounts') renderAccounts();
                else if(item.action === 'reports') renderReports();
                else renderPlaceholder(item.text);
            });

            li.appendChild(a);
            navLinks.appendChild(li);
        });
    }

    function renderPlaceholder(text) {
        managerStats.style.display = 'none';
        contentArea.innerHTML = `
            <div class="glass-card">
                <h3>${text}</h3>
                <p>هذه الواجهة تعرض البيانات المستخرجة من نظام الـ C.</p>
            </div>
        `;
    }

    function renderHome(role) {
        contentArea.innerHTML = `
            <div class="welcome-card glass-card">
                <h3>مرحباً بك في النظام</h3>
                <p>نظام السوبر ماركت المتكامل (C + Web Interface)</p>
            </div>
        `;
        
        if(role === 'manager' && typeof salesStats !== 'undefined') {
            managerStats.style.display = 'grid';
            
            // Update stats from C data
            const cards = managerStats.querySelectorAll('.stat-info p');
            cards[0].textContent = salesStats.totalProducts;
            cards[1].textContent = salesStats.daily.toFixed(2) + " ج.م";
            cards[2].textContent = salesStats.totalCustomers;
            cards[3].textContent = inventoryData.filter(i => isExpired(i.expiry)).length;

            contentArea.appendChild(managerStats);
        } else {
            managerStats.style.display = 'none';
        }
    }

    function isExpired(expiryStr) {
        const [d, m, y] = expiryStr.split('/').map(Number);
        const expiryDate = new Date(y, m - 1, d);
        return expiryDate < new Date();
    }

    function renderStock() {
        managerStats.style.display = 'none';
        if (typeof inventoryData === 'undefined') return;

        let rows = inventoryData.map(item => `
            <tr>
                <td>${item.id}</td>
                <td>${item.name}</td>
                <td>${item.price.toFixed(2)}</td>
                <td>${item.quantity}</td>
                <td>${item.section}</td>
                <td>${item.expiry}</td>
            </tr>
        `).join('');

        contentArea.innerHTML = `
            <div class="table-container">
                <table>
                    <thead>
                        <tr>
                            <th>المعرف</th>
                            <th>الاسم</th>
                            <th>السعر</th>
                            <th>الكمية</th>
                            <th>القسم</th>
                            <th>تاريخ الانتهاء</th>
                        </tr>
                    </thead>
                    <tbody>
                        ${rows}
                    </tbody>
                </table>
            </div>
        `;
    }

    function renderAccounts() {
        managerStats.style.display = 'none';
        if (typeof accountsData === 'undefined') return;

        let rows = accountsData.map(acc => `
            <tr>
                <td>${acc.id}</td>
                <td>${roleMapping[acc.role].title}</td>
                <td>${acc.itemsSold}</td>
                <td>${acc.totalRevenue.toFixed(2)} ج.م</td>
            </tr>
        `).join('');

        contentArea.innerHTML = `
            <div class="table-container">
                <table>
                    <thead>
                        <tr>
                            <th>معرف المستخدم</th>
                            <th>الرتبة</th>
                            <th>القطع المباعة</th>
                            <th>إجمالي الإيرادات</th>
                        </tr>
                    </thead>
                    <tbody>
                        ${rows}
                    </tbody>
                </table>
            </div>
        `;
    }

    function renderReports() {
        managerStats.style.display = 'none';
        if (typeof salesStats === 'undefined') return;

        contentArea.innerHTML = `
            <div class="glass-card">
                <h3>ملخص التقارير المالية</h3>
                <div class="report-details">
                    <p><strong>مبيعات اليوم:</strong> ${salesStats.daily.toFixed(2)} ج.م</p>
                    <p><strong>مبيعات الأسبوع:</strong> ${salesStats.weekly.toFixed(2)} ج.م</p>
                    <p><strong>مبيعات الشهر:</strong> ${salesStats.monthly.toFixed(2)} ج.م</p>
                </div>
            </div>
        `;
    }
});
