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

    // Mock Roles Menu Items
    const menus = {
        manager: [
            { icon: 'fa-home', text: 'الرئيسية', action: 'home' },
            { icon: 'fa-box', text: 'المخزون', action: 'stock' },
            { icon: 'fa-chart-bar', text: 'التقارير', action: 'reports' },
            { icon: 'fa-users', text: 'إدارة الحسابات', action: 'accounts' },
            { icon: 'fa-tags', text: 'الأسعار والعروض', action: 'prices' },
            { icon: 'fa-exclamation-triangle', text: 'المرتجعات المنتهية', action: 'expired' }
        ],
        supervisor: [
            { icon: 'fa-home', text: 'الرئيسية', action: 'home' },
            { icon: 'fa-plus-circle', text: 'إضافة منتج', action: 'add_product' },
            { icon: 'fa-box', text: 'عرض المخزون', action: 'stock' }
        ],
        casher: [
            { icon: 'fa-home', text: 'الرئيسية', action: 'home' },
            { icon: 'fa-shopping-cart', text: 'نقطة البيع (POS)', action: 'sell' },
            { icon: 'fa-file-invoice-dollar', text: 'آخر فاتورة', action: 'bill' },
            { icon: 'fa-box', text: 'استعلام مخزون', action: 'stock' }
        ],
        customer: [
            { icon: 'fa-home', text: 'الرئيسية', action: 'home' },
            { icon: 'fa-store', text: 'تصفح المنتجات', action: 'browse' },
            { icon: 'fa-shopping-basket', text: 'عربة التسوق', action: 'cart' },
            { icon: 'fa-undo', text: 'إرجاع منتج منتهي', action: 'return' }
        ]
    };

    // Determine Role based on ID
    function getRole(id) {
        if(id === '#001') return { role: 'manager', title: 'المدير' };
        if(id.startsWith('#S')) return { role: 'supervisor', title: 'مشرف' };
        if(id.startsWith('#C')) return { role: 'casher', title: 'كاشير' };
        return { role: 'customer', title: 'عميل' };
    }

    // Login Handler
    loginForm.addEventListener('submit', (e) => {
        e.preventDefault();
        const id = userIdInput.value.trim();
        if(!id) return;

        const user = getRole(id);
        
        // Update UI
        userRoleTitle.textContent = user.title;
        userIdDisplay.textContent = id;
        
        // Render Menu
        renderMenu(user.role);

        // Show Dashboard
        loginScreen.classList.remove('active');
        dashboardScreen.classList.add('active');

        // Show specific dashboard widgets based on role
        if(user.role === 'manager') {
            managerStats.style.display = 'grid';
            pageTitle.textContent = 'نظرة عامة - المدير';
        } else {
            managerStats.style.display = 'none';
            pageTitle.textContent = `لوحة تحكم - ${user.title}`;
        }
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
                // Remove active from all
                document.querySelectorAll('.nav-links a').forEach(el => el.classList.remove('active'));
                a.classList.add('active');
                
                // Demo interactions
                pageTitle.textContent = item.text;
                if(item.action === 'stock') renderMockStock();
                else if(item.action === 'home') renderHome(role);
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
                <p>هذه واجهة تجريبية. سيتم ربط هذه الشاشة بوظائف نظام الـ C لاحقاً.</p>
            </div>
        `;
    }

    function renderHome(role) {
        contentArea.innerHTML = `
            <div class="welcome-card glass-card">
                <h3>مرحباً بك في النظام</h3>
                <p>اختر إجراء من القائمة الجانبية للبدء.</p>
            </div>
        `;
        if(role === 'manager') {
            managerStats.style.display = 'grid';
            contentArea.appendChild(managerStats);
        }
    }

    function renderMockStock() {
        managerStats.style.display = 'none';
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
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                            <td>101</td>
                            <td>حليب مراعي 1 لتر</td>
                            <td>25.00</td>
                            <td>50</td>
                            <td>الألبان</td>
                        </tr>
                        <tr>
                            <td>102</td>
                            <td>خبز أبيض</td>
                            <td>10.00</td>
                            <td>100</td>
                            <td>المخبوزات</td>
                        </tr>
                        <tr>
                            <td>103</td>
                            <td>جبنة شيدر 250ج</td>
                            <td>45.00</td>
                            <td>30</td>
                            <td>الألبان</td>
                        </tr>
                    </tbody>
                </table>
            </div>
        `;
    }
});
